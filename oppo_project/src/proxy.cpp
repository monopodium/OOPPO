#include "proxy.h"
#include "jerasure.h"
#include "reed_sol.h"
#include "tinyxml2.h"
#include <thread>
#include <cassert>
#include <string>
template <typename T> inline T ceil(T const &A, T const &B) {
  return T((A + B - 1) / B);
};
namespace OppoProject {
grpc::Status ProxyImpl::checkalive(grpc::ServerContext *context,
                                   const proxy_proto::CheckaliveCMD *request,
                                   proxy_proto::RequestResult *response) {

  std::cout << "checkalive" << request->name() << std::endl;
  response->set_message(false);
  init_coordinator();
  return grpc::Status::OK;
}
bool ProxyImpl::SetToMemcached(const char *key, size_t key_length,
                               const char *value, size_t value_length, const char *ip, int port) {
  memcached_return rc;
  memcached_lock.lock();
  rc = memcached_set_by_ip_and_port(m_memcached, key, key_length, value, value_length,
                     (time_t)0, (uint32_t)0, ip, port);
  memcached_lock.unlock();
  std::cout << "set " << std::string(key) << " " << std::string(ip) << " " << port << " " << (MEMCACHED_SUCCESS==rc) << " " << value_length << std::endl;
  return true;
}
bool ProxyImpl::GetFromMemcached(const char *key, size_t key_length,
                                 char *value, size_t *value_length, const char *ip, int port) {
  memcached_return rc;
  memcached_return_t error;
  uint32_t flag;

  memcached_lock.lock();
  char *value_ptr = memcached_get_by_ip_and_port(m_memcached, key, key_length, value_length, &flag, &error, ip, port);
  memcached_lock.unlock();
  std::cout << "get " << std::string(key) << " " << std::string(ip) << " " << port << " " << *value_length << " " << (value_ptr==nullptr) << std::endl;

  if (value_ptr != NULL) {
    memcpy(value, value_ptr, int(*value_length));
    return true;
  } else {
    return false;
  }
}
bool encode(int k, int m, int l, char **data_ptrs, char **coding_ptrs, int blocksize, EncodeType encode_type) {
  int *matrix;
  matrix = reed_sol_vandermonde_coding_matrix(k, m, 8);
  if (encode_type == RS) {
    jerasure_matrix_encode(k, m, 8, matrix, data_ptrs, coding_ptrs, blocksize);
  } else if (encode_type == Azure_LRC_1 || encode_type == OPPO_LRC) {
    std::vector<int> new_matrix((m+l)*k, 1);
    memcpy(new_matrix.data(), matrix, m*k*sizeof(int));
    jerasure_matrix_encode(k, m + l, 8, new_matrix.data(), data_ptrs, coding_ptrs, blocksize);
    std::vector<char> last_local(blocksize, 0);
    std::vector<char *> v_temp_coding(1);
    char **temp_coding = v_temp_coding.data();
    temp_coding[0] = last_local.data();
    std::vector<int> last_matrix(m, 1);
    jerasure_matrix_encode(m, 1, 8, last_matrix.data(), coding_ptrs, temp_coding, blocksize);
    memcpy(coding_ptrs[m+l], last_local.data(), blocksize);
  }
  return true;
}

bool decode(int k, int m, char **data_ptrs, char **coding_ptrs, int *erasures, int blocksize) {
  int *matrix;
  matrix = reed_sol_vandermonde_coding_matrix(k, m, 8);
  jerasure_matrix_decode(k, m, 8, matrix, 0, erasures, data_ptrs, coding_ptrs, blocksize);
  return true;
}

grpc::Status ProxyImpl::EncodeAndSetObject(
    grpc::ServerContext *context,
    const proxy_proto::ObjectAndPlacement *object_and_placement,
    proxy_proto::SetReply *response) {

  std::string key = object_and_placement->key();
  bool big = object_and_placement->bigobject();
  int value_size_bytes = object_and_placement->valuesizebyte();
  int k = object_and_placement->k();
  int m = object_and_placement->m();
  int l = object_and_placement->l();
  int shard_size = object_and_placement->shard_size();
  int tail_shard_size = object_and_placement->tail_shard_size();
  OppoProject::EncodeType encode_type = (OppoProject::EncodeType)object_and_placement->encode_type();
  std::vector<unsigned int> stripe_ids;
  for (int i = 0; i < object_and_placement->stripe_ids_size(); i++) {
    stripe_ids.push_back(object_and_placement->stripe_ids(i));
  }
  std::vector<std::pair<std::string, int>> nodes_ip_and_port;
  for (int i = 0; i < object_and_placement->datanodeip_size(); i++) {
    nodes_ip_and_port.push_back(std::make_pair(object_and_placement->datanodeip(i), object_and_placement->datanodeport(i)));
  }

  auto encode_and_save = [this, big, key, value_size_bytes, k, m, l, shard_size, tail_shard_size, stripe_ids, nodes_ip_and_port, encode_type]() mutable {
    if (big == true) {
      asio::ip::tcp::socket socket_data(io_context);
      acceptor.accept(socket_data);
      asio::error_code error;

      int extend_value_size_byte = shard_size * k * stripe_ids.size();
      std::vector<char> buf_key(key.size());
      std::vector<char> v_buf(extend_value_size_byte);
      for (int i = value_size_bytes; i < extend_value_size_byte; i++) {
        v_buf[i] = '0';
      }
      size_t len = asio::read(socket_data, asio::buffer(buf_key, key.size()), error);
      if (error == asio::error::eof) {
        std::cout << "error == asio::error::eof" << std::endl;
      } else if (error) {
        throw asio::system_error(error);
      }
      int flag = 1;
      for (int i = 0; i < key.size(); i++) {
        if (key[i] != buf_key[i]) {
          flag = 0;
        }
      }
      if (flag) {
        len = asio::read(socket_data, asio::buffer(v_buf, value_size_bytes), error);
      }
      socket_data.shutdown(asio::ip::tcp::socket::shutdown_receive);
      socket_data.close();

      char *buf = v_buf.data();
      for (int i = 0; i < stripe_ids.size(); i++) {
        std::vector<char *> v_data(k);
        std::vector<char *> v_coding(m + l + 1);
        char **data = (char **)v_data.data();
        char **coding = (char **)v_coding.data();
        if ((i == stripe_ids.size() - 1) && (tail_shard_size != -1)) {
          std::vector<std::vector<char>> v_coding_area(m + l + 1, std::vector<char>(tail_shard_size));
          for (int j = 0; j < k; j++) {
            data[j] = &buf[j * tail_shard_size];
          }
          for (int j = 0; j < m + l + 1; j++) {
            coding[j] = v_coding_area[j].data();
          }
          int send_num;
          if (encode_type == RS) {
            encode(k, m, 0, data, coding, tail_shard_size, encode_type);
            send_num = k + m;
          } else if (encode_type == OPPO_LRC || encode_type == Azure_LRC_1) {
            encode(k, m, l, data, coding, tail_shard_size, encode_type);
            send_num = k + m + l + 1;
          }
          for (int j = 0; j < send_num; j++) {
            std::string shard_id = std::to_string(stripe_ids[i] * 1000 + j);
            std::pair<std::string, int> &ip_and_port = nodes_ip_and_port[j];
            if (j < k) {
              SetToMemcached(shard_id.c_str(), shard_id.size(), data[j], tail_shard_size, ip_and_port.first.c_str(), ip_and_port.second);
            } else {
              SetToMemcached(shard_id.c_str(), shard_id.size(), coding[j-k], tail_shard_size, ip_and_port.first.c_str(), ip_and_port.second);
            }
          }
        } else {
          std::vector<std::vector<char>> v_coding_area(m + l + 1, std::vector<char>(shard_size));
          for (int j = 0; j < k; j++) {
            data[j] = &buf[j * shard_size];
          }
          for (int j = 0; j < m + l + 1; j++) {
            coding[j] = v_coding_area[j].data();
          }
          int send_num = 0;
          if (encode_type == RS) {
            encode(k, m, 0, data, coding, shard_size, encode_type);
            send_num = k + m;
          } else if (encode_type == OPPO_LRC || encode_type == Azure_LRC_1) {
            encode(k, m, l, data, coding, shard_size, encode_type);
            send_num = k + m + l + 1;
          }
          for (int j = 0; j < send_num; j++) {
            std::pair<std::string, int> &ip_and_port = nodes_ip_and_port[j];
            std::string shard_id = std::to_string(stripe_ids[i] * 1000 + j);
            if (j < k) {
              SetToMemcached(shard_id.c_str(), shard_id.size(), data[j], shard_size, ip_and_port.first.c_str(), ip_and_port.second);
            } else {
              SetToMemcached(shard_id.c_str(), shard_id.size(), coding[j-k], shard_size, ip_and_port.first.c_str(), ip_and_port.second);
            }
          }
        }
        buf += (k * shard_size);
      }

      coordinator_proto::CommitAbortKey commit_abort_key;
      coordinator_proto::ReplyFromCoordinator result;
      grpc::ClientContext context;
      commit_abort_key.set_key(key);
      commit_abort_key.set_ifcommitmetadata(true);
      grpc::Status status;
      status = this->m_coordinator_stub->reportCommitAbort(
          &context, commit_abort_key, &result);
      if (status.ok()) {
        std::cout << "connect coordinator,ok" << std::endl;
      } else {
        std::cout << "oooooH coordinator,fail!!!!" << std::endl;
      }
    }
  };
  try {
    std::thread my_thread(encode_and_save);
    my_thread.detach();
  } catch (std::exception &e) {
    std::cout << "exception" << std::endl;
    std::cout << e.what() << std::endl;
  }
  std::cout << "receive askDNhandling rpc!\n";

  return grpc::Status::OK;
}

grpc::Status ProxyImpl::decodeAndGetObject(
    grpc::ServerContext *context,
    const proxy_proto::ObjectAndPlacement *object_and_placement,
    proxy_proto::GetReply *response) {
  std::string key = object_and_placement->key();
  bool big = object_and_placement->bigobject();
  int k = object_and_placement->k();
  int m = object_and_placement->m();
  int shard_size = object_and_placement->shard_size();
  int tail_shard_size = object_and_placement->tail_shard_size();
  int value_size_bytes = object_and_placement->valuesizebyte();
  std::string clientip = object_and_placement->clientip();
  int clientport = object_and_placement->clientport();
  std::vector<unsigned int> stripe_ids;
  for (int i = 0; i < object_and_placement->stripe_ids_size(); i++) {
    stripe_ids.push_back(object_and_placement->stripe_ids(i));
  }
  std::vector<std::pair<std::string, int>> nodes_ip_and_port;
  for (int i = 0; i < object_and_placement->datanodeip_size(); i++) {
    nodes_ip_and_port.push_back({object_and_placement->datanodeip(i), object_and_placement->datanodeport(i)});
  }
  auto decode_and_get = [this, big, key, k, m, shard_size, tail_shard_size, value_size_bytes,
                         clientip, clientport, stripe_ids, nodes_ip_and_port]() mutable {
    if (big) {
      std::string value;
      for (int i = 0; i < stripe_ids.size(); i++) {
        auto shards_ptr = std::make_shared<std::vector<std::vector<char>>>();
        auto shards_idx_ptr = std::make_shared<std::vector<int>>();
        auto myLock_ptr = std::make_shared<std::mutex>();
        auto cv_ptr = std::make_shared<std::condition_variable>();
        std::vector<char *> v_data(k);
        std::vector<char *> v_coding(m);
        char **data = v_data.data();
        char **coding = v_coding.data();
        auto getFromNode = [this, k, shards_ptr, shards_idx_ptr, myLock_ptr, cv_ptr](int stripe_id, int shard_idx, int x_shard_size, std::string ip, int port) {
          std::string shard_id = std::to_string(stripe_id * 1000 + shard_idx);
          std::vector<char> temp(x_shard_size);
          size_t temp_size;
          bool ret = GetFromMemcached(shard_id.c_str(), shard_id.size(), temp.data(), &temp_size, ip.c_str(), port);
          if (!ret) {
            return;
          }
          myLock_ptr->lock();
          if (shards_ptr->size() < k && shards_idx_ptr->size() < k) {
            shards_ptr->push_back(temp);
            shards_idx_ptr->push_back(shard_idx);
            if (shards_ptr->size() == k && shards_idx_ptr->size() == k) {
              cv_ptr->notify_all();
            }
          }
          myLock_ptr->unlock();
        };
        if ((i == stripe_ids.size() - 1) && tail_shard_size != -1) {
          std::vector<std::vector<char>> v_data_area(k, std::vector<char>(tail_shard_size));
          std::vector<std::vector<char>> v_coding_area(m, std::vector<char>(tail_shard_size));
          for (int j = 0; j < k; j++) {
            data[j] = v_data_area[j].data();
          }
          for (int j = 0; j < m; j++) {
            coding[j] = v_coding_area[j].data();
          }
          std::vector<std::thread> read_memcached_treads;
          for (int j = 0; j < k+m; j++) {
            std::pair<std::string, int> &ip_and_port = nodes_ip_and_port[j];
            read_memcached_treads.push_back(std::thread(
              getFromNode, stripe_ids[i], j, tail_shard_size, ip_and_port.first, ip_and_port.second)
            );
          }
          for (int j = 0; j < k+m; j++) {
            read_memcached_treads[j].detach();
          }
          std::unique_lock<std::mutex> lck(*myLock_ptr);
          while (!(shards_ptr->size() == k && shards_idx_ptr->size() == k)) {
            cv_ptr->wait(lck);
          }
          for (int j = 0; j < shards_idx_ptr->size(); j++) {
            int idx = (*shards_idx_ptr)[j];
            if (idx < k) {
              memcpy(data[idx], (*shards_ptr)[j].data(), tail_shard_size);
            } else {
              memcpy(coding[idx - k], (*shards_ptr)[j].data(), tail_shard_size);
            }
          }
          std::vector<int> erasures;
          for (int j = 0; j < k + m; j++) {
            if (std::find(shards_idx_ptr->begin(), shards_idx_ptr->end(), j) == shards_idx_ptr->end()) {
              erasures.push_back(j);
            }
          }
          erasures.push_back(-1);
          decode(k, m, data, coding, erasures.data(), tail_shard_size);
          for (int j = 0; j < k; j++) {
            value += std::string(data[j], tail_shard_size);
          }
        } else {
          std::vector<std::vector<char>> v_data_area(k, std::vector<char>(shard_size));
          std::vector<std::vector<char>> v_coding_area(m, std::vector<char>(shard_size));
          for (int j = 0; j < k; j++) {
            data[j] = v_data_area[j].data();
          }
          for (int j = 0; j < m; j++) {
            coding[j] = v_coding_area[j].data();
          }
          std::vector<std::thread> read_memcached_treads;
          for (int j = 0; j < k+m; j++) {
            std::pair<std::string, int> &ip_and_port = nodes_ip_and_port[j];
            read_memcached_treads.push_back(std::thread(
              getFromNode, stripe_ids[i], j, shard_size, ip_and_port.first, ip_and_port.second)
            );
          }
          for (int j = 0; j < k+m; j++) {
            read_memcached_treads[j].detach();
          }
          std::unique_lock<std::mutex> lck(*myLock_ptr);
          while (!(shards_ptr->size() == k && shards_idx_ptr->size() == k)) {
            cv_ptr->wait(lck);
          }
          for (int j = 0; j < shards_idx_ptr->size(); j++) {
            int idx = (*shards_idx_ptr)[j];
            if (idx < k) {
              memcpy(data[idx], (*shards_ptr)[j].data(), shard_size);
            } else {
              memcpy(coding[idx - k], (*shards_ptr)[j].data(), shard_size);
            }
          }
          std::vector<int> erasures;
          for (int j = 0; j < k + m; j++) {
            if (std::find(shards_idx_ptr->begin(), shards_idx_ptr->end(), j) == shards_idx_ptr->end()) {
              erasures.push_back(j);
            }
          }
          erasures.push_back(-1);
          decode(k, m, data, coding, erasures.data(), shard_size);
          for (int j = 0; j < k; j++) {
            value += std::string(data[j], shard_size);
          }
        }
      }
      asio::io_context io_context;
      asio::error_code error;
      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::resolver::results_type endpoints =
          resolver.resolve(clientip, std::to_string(clientport));

      asio::ip::tcp::socket sock_data(io_context);
      asio::connect(sock_data, endpoints);

      std::cout << "key.size()" << key.size() << std::endl;
      std::cout << "value.size()" << value.size() << std::endl;
      asio::write(sock_data, asio::buffer(key, key.size()), error);
      asio::write(sock_data, asio::buffer(value, value_size_bytes), error);
      sock_data.shutdown(asio::ip::tcp::socket::shutdown_send);
      sock_data.close();
    }
  };
  try {
    std::thread my_thread(decode_and_get);
    my_thread.detach();
  } catch (std::exception &e) {
    std::cout << "exception" << std::endl;
    std::cout << e.what() << std::endl;
  }

  return grpc::Status::OK;
}

bool ProxyImpl::init_coordinator() {
  std::string coordinator_ip_port = "0.0.0.0:55555";
  m_coordinator_stub =
      coordinator_proto::CoordinatorService::NewStub(grpc::CreateChannel(
          coordinator_ip_port, grpc::InsecureChannelCredentials()));
  return true;
}

bool ProxyImpl::init_memcached() {
  /*这里原本是写死的datanode的ip和port，需要改成从.xml文件中读取*/
  memcached_return rc;
  memcached_server_st *servers;
  m_memcached = memcached_create(NULL);

  bool init = false;
  tinyxml2::XMLDocument xml;
  xml.LoadFile(config_path.c_str());
  tinyxml2::XMLElement *root = xml.RootElement();
  for (tinyxml2::XMLElement* az = root->FirstChildElement(); az != nullptr; az = az->NextSiblingElement()) {
    for (tinyxml2::XMLElement* node = az->FirstChildElement()->FirstChildElement(); node != nullptr; node = node->NextSiblingElement()) {
      std::string node_uri(node->Attribute("uri"));
      auto pos = node_uri.find(':');
      std::string Node_ip = node_uri.substr(0, pos);
      int Node_port = std::stoi(node_uri.substr(pos+1, node_uri.size()));
      std::cout << Node_ip << ":" << Node_port << std::endl;
      if (!init) {
        servers = memcached_server_list_append(NULL, Node_ip.c_str(), Node_port, &rc);
        init = true;
      } else {
        servers = memcached_server_list_append(servers, Node_ip.c_str(), Node_port, &rc);
      }
    }
  }

  rc = memcached_server_push(m_memcached, servers);
  memcached_server_free(servers);

  memcached_behavior_set(m_memcached, MEMCACHED_BEHAVIOR_DISTRIBUTION,
                         MEMCACHED_DISTRIBUTION_CONSISTENT);
  memcached_behavior_set(m_memcached, MEMCACHED_BEHAVIOR_RETRY_TIMEOUT, 20);
  //  memcached_behavior_set(m_memcached,
  //  MEMCACHED_BEHAVIOR_REMOVE_FAILED_SERVERS, 1) ;  //
  //  同时设置MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT 和
  //  MEMCACHED_BEHAVIOR_AUTO_EJECT_HOSTS
  memcached_behavior_set(m_memcached, MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT,
                         5);
  memcached_behavior_set(m_memcached, MEMCACHED_BEHAVIOR_AUTO_EJECT_HOSTS,
                         true);
}
} // namespace OppoProject
