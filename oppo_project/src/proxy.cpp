#include "proxy.h"
#include "jerasure.h"
#include "reed_sol.h"

#include <thread>

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
                               const char *value, size_t value_length) {
  memcached_return rc;
  rc = memcached_set(m_memcached, key, key_length, value, value_length,
                     (time_t)0, (uint32_t)0);
  return true;
}
bool ProxyImpl::GetFromMemcached(const char *key, size_t key_length,
                                 char **value, size_t *value_length) {
  memcached_return rc;
  memcached_return_t error;
  uint32_t flag;

  char *value_ptr =
      memcached_get(m_memcached, key, key_length, value_length, &flag, &error);

  if (value_ptr != NULL) {
    std::cout << "find " << key << " value_length:" << *value_length
              << std::endl;
    uint32_t j = 0;

    *value = (char *)malloc(sizeof(char) * int(*value_length));
    std::cout << "jjjjj" << std::endl;
    memcpy(*value, value_ptr, int(*value_length));
    // for (j = 0; j < *value_length; j++) {
    //   std::cout << (*value)[j];
    // }
    std::cout << std::endl;
  } else {
    std::cout << "can not find " << key << std::endl;
  }
  return true;
}
bool encode(int k, int m, char **data_ptrs, char **coding_ptrs, int blocksize) {
  int *matrix;
  matrix = reed_sol_vandermonde_coding_matrix(k, m, 8);
  jerasure_matrix_encode(k, m, 8, matrix, data_ptrs, coding_ptrs, blocksize);
  return true;
}
grpc::Status ProxyImpl::EncodeAndSetObject(
    grpc::ServerContext *context,
    const proxy_proto::ObjectAndPlacement *object_and_placement,
    proxy_proto::SetReply *response) {

  std::string key = object_and_placement->key();
  int value_size_bytes = object_and_placement->valuesizebyte();
  int k = object_and_placement->k();
  int m = object_and_placement->m();
  int blocksizebyte = object_and_placement->blocksizebyte();
  std::vector<int> shard_id_list;
  shard_id_list.assign(object_and_placement->shardid().cbegin(),
                       object_and_placement->shardid().cend());

  for (auto shardid = object_and_placement->shardid().cbegin();
       shardid != object_and_placement->shardid().cend(); shardid++) {
    std::cout << *shardid << std::endl;
  }
  for (int i = 0; i < shard_id_list.size(); i++) {
    std::cout << "shard_id_list[i]:" << shard_id_list[i] << std::endl;
  }

  auto encode_and_save = [this, key, value_size_bytes, k, m, blocksizebyte,
                          shard_id_list]() mutable {
    asio::io_context io_context;
    asio::ip::tcp::acceptor acceptor(
        io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 12233));
    asio::ip::tcp::socket socket_data(io_context);
    acceptor.accept(socket_data);
    asio::error_code error;

    std::cout << "key.size() " << key.size() << std::endl;

    int extend_value_size_byte =
        blocksizebyte * ceil(value_size_bytes, blocksizebyte);
    std::cout << "extend_value_size_byte:" << extend_value_size_byte
              << std::endl;
    std::vector<char> buf_key(key.size());
    std::vector<char> buf(extend_value_size_byte);

    for (int i = value_size_bytes; i < extend_value_size_byte; i++) {
      buf[i] = '0';
    }
    size_t len = asio::read(socket_data, asio::buffer(buf_key, key.size()), error);

    if (error == asio::error::eof) {
      std::cout << "error == asio::error::eof" << std::endl;
    } else if (error) {
      throw asio::system_error(error);
    }
    for (int i = 0; i < key.size(); i++) {
      std::cout << buf_key[i];
    }
    std::cout << std::endl;
    int flag = 1;
    for (int i = 0; i < key.size(); i++) {
      if (key[i] != buf_key[i]) {
        flag = 0;
      }
    }
    if (flag) {
      len = asio::read(socket_data, asio::buffer(buf, value_size_bytes), error);
      std::cout << len << std::endl;
    }
    for (const auto &c : buf) {
      std::cout << c;
    }
    std::cout << std::endl;
    // this->


    std::vector<char *> v_data(k);
    std::vector<char *> v_coding(m);
    std::vector<std::vector<char>> v_coding_area(m, std::vector<char>(blocksizebyte));
    /*将块切分！*/
    char **data = (char **)v_data.data();
    char **coding = (char **)v_coding.data();

    for (int i = 0; i * blocksizebyte < extend_value_size_byte; i = i + 1) {
      data[i] = &buf[i * blocksizebyte];
    }

    for (int i = 0; i < m; i++) {
      coding[i] = v_coding_area[i].data();
    }

    encode(k, m, data, coding, blocksizebyte);

    for (int i = 0; i < blocksizebyte; i++) {
      std::cout << coding[0][i];
    }
    std::cout << std::endl;

    for (int i = 0; i < k + m; i++) {
      std::string shardid_str = std::to_string(shard_id_list[i]);
      // char *shardid_str = (char *)malloc(sizeof(char) * 64);
      // s(object_and_placement->shardid()[i], shardid_str, 10);
      unsigned int a;
      std::cout << (shard_id_list[i]) << std::endl;
      std::cout << shardid_str << std::endl;
      if (i < k) {
        SetToMemcached(shardid_str.c_str(), shardid_str.size(), data[i],
                       blocksizebyte);
      } else {
        SetToMemcached(shardid_str.c_str(), shardid_str.size(), coding[i - k],
                       blocksizebyte);
      }
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
  int k = object_and_placement->k();
  int m = object_and_placement->m();
  int blocksizebyte = object_and_placement->blocksizebyte();
  int value_size_bytes = object_and_placement->valuesizebyte();
  std::string clientip = object_and_placement->clientip();
  int clientport = object_and_placement->clientport();
  std::vector<int> shardid;
  shardid.assign(object_and_placement->shardid().cbegin(),
                 object_and_placement->shardid().cend());
  auto decode_and_get = [this, key, k, m, blocksizebyte, value_size_bytes,
                         clientip, clientport, shardid]() mutable {
    std::cout << "key:" << key << std::endl;
    std::cout << "k:" << k << std::endl;
    std::cout << "m:" << m << std::endl;
    std::cout << "blocksizebyte:" << blocksizebyte << std::endl;
    std::cout << "value_size_bytes" << value_size_bytes << std::endl;
    std::cout << "clientip" << clientip << std::endl;
    std::cout << "clientport" << clientport << std::endl;
    for (int i = 0; i < shardid.size(); i++) {
      std::cout << "shardid[i]:" << shardid[i] << std::endl;
    }
    char **data = (char **)malloc(sizeof(char *) * k);
    char **coding = (char **)malloc(sizeof(char *) * m);
    std::cout << std::endl;
    std::string value;
    for (int i = 0; i < k; i++) {
      std::cout << shardid[i] << std::endl;
      size_t value_length = 1;
      std::string shardid_str = std::to_string(shardid[i]);
      GetFromMemcached(shardid_str.c_str(), shardid_str.size(), &data[i],
                       &value_length);
      // for (int j = 0; j < value_length; j++) {
      //   std::cout << data[i][j];
      // }
    }

    for (int i = 0; i < k; i++) {
      value = value + std::string(data[i], blocksizebyte);
    }
    std::cout << "value: " << value << std::endl;
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
  std::string coordinator_ip_port = "localhost:50051";
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

  servers = memcached_server_list_append(NULL, (char *)"localhost", 8100, &rc);
  servers =
      memcached_server_list_append(servers, (char *)"localhost", 8101, &rc);
  servers =
      memcached_server_list_append(servers, (char *)"localhost", 8102, &rc);
  servers =
      memcached_server_list_append(servers, (char *)"localhost", 8103, &rc);
  servers =
      memcached_server_list_append(servers, (char *)"localhost", 8104, &rc);
  servers =
      memcached_server_list_append(servers, (char *)"localhost", 8105, &rc);
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
