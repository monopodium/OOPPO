#include "proxy.h"
#include "jerasure.h"
#include "reed_sol.h"
#include "tinyxml2.h"
#include "toolbox.h"
#include "azure_lrc.h"
#include <thread>
#include <cassert>
#include <string>
template <typename T>
inline T ceil(T const &A, T const &B)
{
  return T((A + B - 1) / B);
};
namespace OppoProject
{
  grpc::Status ProxyImpl::checkalive(grpc::ServerContext *context,
                                     const proxy_proto::CheckaliveCMD *request,
                                     proxy_proto::RequestResult *response)
  {

    std::cout << "checkalive" << request->name() << std::endl;
    response->set_message(false);
    init_coordinator();
    return grpc::Status::OK;
  }
  bool ProxyImpl::SetToMemcached(const char *key, size_t key_length,
                                 const char *value, size_t value_length, const char *ip, int port)
  {
    try
    {
      std::cout << "SetToMemcached" << " " << std::string(ip) << " " << port << std::endl;
      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(std::string(ip), std::to_string(port));
      asio::ip::tcp::socket socket(io_context);
      asio::connect(socket, endpoint);

      int flag = 0;
      std::vector<unsigned char> int_buf_flag = OppoProject::int_to_bytes(flag);
      asio::write(socket, asio::buffer(int_buf_flag, int_buf_flag.size()));

      std::vector<unsigned char> int_buf_key_size = OppoProject::int_to_bytes(key_length);
      asio::write(socket, asio::buffer(int_buf_key_size, int_buf_key_size.size()));

      std::vector<unsigned char> int_buf_value_size = OppoProject::int_to_bytes(value_length);
      asio::write(socket, asio::buffer(int_buf_value_size, int_buf_value_size.size()));

      asio::write(socket, asio::buffer(key, key_length));
      asio::write(socket, asio::buffer(value, value_length));

      std::vector<char> finish(1);
      asio::read(socket, asio::buffer(finish, finish.size()));

      socket.shutdown(asio::ip::tcp::socket::shutdown_both);
      socket.close();
    }
    catch (std::exception &e)
    {
      std::cout << e.what() << std::endl;
    }

    // std::cout << "set " << std::string(key) << " " << std::string(ip) << " " << port << " " << (MEMCACHED_SUCCESS==rc) << " " << value_length << std::endl;
    return true;
  }
  bool ProxyImpl::GetFromMemcached(const char *key, size_t key_length,
                                   char *value, size_t *value_length, int offset, int lenth, const char *ip, int port)
  {
    try
    {
      std::cout << "GetFromMemcached" << " " << std::string(ip) << " " << port << std::endl;
      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(std::string(ip), std::to_string(port));
      asio::ip::tcp::socket socket(io_context);
      asio::connect(socket, endpoint);

      int flag = 1;
      std::vector<unsigned char> int_buf_flag = OppoProject::int_to_bytes(flag);
      asio::write(socket, asio::buffer(int_buf_flag, int_buf_flag.size()));

      std::vector<unsigned char> int_buf_key_size = OppoProject::int_to_bytes(key_length);
      asio::write(socket, asio::buffer(int_buf_key_size, int_buf_key_size.size()));

      asio::write(socket, asio::buffer(key, key_length));

      std::vector<unsigned char> int_buf_offset = OppoProject::int_to_bytes(offset);
      asio::write(socket, asio::buffer(int_buf_offset, int_buf_offset.size()));

      std::vector<unsigned char> int_buf_lenth = OppoProject::int_to_bytes(lenth);
      asio::write(socket, asio::buffer(int_buf_lenth, int_buf_lenth.size()));

      std::vector<unsigned char> value_from_datanode(lenth);

      asio::read(socket, asio::buffer(value_from_datanode, value_from_datanode.size()));

      socket.shutdown(asio::ip::tcp::socket::shutdown_both);
      socket.close();

      memcpy(value, value_from_datanode.data(), lenth);
    }
    catch (std::exception &e)
    {
      std::cout << e.what() << std::endl;
    }

    return true;
  }

  grpc::Status ProxyImpl::EncodeAndSetObject(
      grpc::ServerContext *context,
      const proxy_proto::ObjectAndPlacement *object_and_placement,
      proxy_proto::SetReply *response)
  {

    std::string key = object_and_placement->key();
    bool big = object_and_placement->bigobject();
    int value_size_bytes = object_and_placement->valuesizebyte();
    int k = object_and_placement->k();
    int m = object_and_placement->m();
    int real_l = object_and_placement->real_l();
    int shard_size = object_and_placement->shard_size();
    int tail_shard_size = object_and_placement->tail_shard_size();
    OppoProject::EncodeType encode_type = (OppoProject::EncodeType)object_and_placement->encode_type();
    std::vector<unsigned int> stripe_ids;
    for (int i = 0; i < object_and_placement->stripe_ids_size(); i++)
    {
      stripe_ids.push_back(object_and_placement->stripe_ids(i));
    }
    std::vector<std::pair<std::string, int>> nodes_ip_and_port;
    for (int i = 0; i < object_and_placement->datanodeip_size(); i++)
    {
      nodes_ip_and_port.push_back(std::make_pair(object_and_placement->datanodeip(i), object_and_placement->datanodeport(i)));
    }
    auto encode_and_save = [this, big, key, value_size_bytes, k, m, real_l, shard_size, tail_shard_size, stripe_ids, nodes_ip_and_port, encode_type]() mutable
    {
      try
      {
        if (big == true)
        {
          asio::ip::tcp::socket socket_data(io_context);
          acceptor.accept(socket_data);
          asio::error_code error;

          int extend_value_size_byte = shard_size * k * stripe_ids.size();
          std::vector<char> buf_key(key.size());
          std::vector<char> v_buf(extend_value_size_byte);
          for (int i = value_size_bytes; i < extend_value_size_byte; i++)
          {
            v_buf[i] = '0';
          }
          // size_t len = asio::read(socket_data, asio::buffer(buf_key, key.size()), error);
          asio::read(socket_data, asio::buffer(buf_key, key.size()), error);
          if (error == asio::error::eof)
          {
            std::cout << "error == asio::error::eof" << std::endl;
          }
          else if (error)
          {
            throw asio::system_error(error);
          }
          int flag = 1;
          for (int i = 0; i < int(key.size()); i++)
          {
            if (key[i] != buf_key[i])
            {
              flag = 0;
            }
          }
          if (flag)
          {
            // len = asio::read(socket_data, asio::buffer(v_buf, value_size_bytes), error);
            asio::read(socket_data, asio::buffer(v_buf, value_size_bytes), error);
          }
          socket_data.shutdown(asio::ip::tcp::socket::shutdown_receive);
          socket_data.close();

          char *buf = v_buf.data();
          auto send_to_datanode = [this](int j, int k, std::string shard_id, char **data, char **coding, int x_shard_size, std::pair<std::string, int> ip_and_port)
          {
            if (j < k)
            {
              SetToMemcached(shard_id.c_str(), shard_id.size(), data[j], x_shard_size, ip_and_port.first.c_str(), ip_and_port.second);
            }
            else
            {
              SetToMemcached(shard_id.c_str(), shard_id.size(), coding[j - k], x_shard_size, ip_and_port.first.c_str(), ip_and_port.second);
            }
          };
          for (int i = 0; i < int(stripe_ids.size()); i++)
          {
            std::vector<char *> v_data(k);
            std::vector<char *> v_coding(m + real_l + 1);
            char **data = (char **)v_data.data();
            char **coding = (char **)v_coding.data();
            int true_shard_size;
            if ((i == int(stripe_ids.size() - 1)) && tail_shard_size != -1)
            {
              true_shard_size = tail_shard_size;
            }
            else
            {
              true_shard_size = shard_size;
            }
            std::vector<std::vector<char>> v_coding_area(m + real_l + 1, std::vector<char>(true_shard_size));
            for (int j = 0; j < k; j++)
            {
              data[j] = &buf[j * true_shard_size];
            }
            for (int j = 0; j < m + real_l + 1; j++)
            {
              coding[j] = v_coding_area[j].data();
            }
            int send_num;
            if (encode_type == RS)
            {
              encode(k, m, 0, data, coding, true_shard_size, encode_type);
              send_num = k + m;
            }
            else if (encode_type == Azure_LRC_1)
            {
              // m = g for lrc
              encode(k, m, real_l, data, coding, true_shard_size, encode_type);
              send_num = k + m + real_l + 1;
            }
            else if (encode_type == OPPO_LRC)
            {
              encode(k, m, real_l, data, coding, true_shard_size, encode_type);
              send_num = k + m + real_l;
            }
            std::vector<std::thread> senders;

            for (int j = 0; j < send_num; j++)
            {
              std::string shard_id = std::to_string(stripe_ids[i] * 1000 + j);
              std::pair<std::string, int> &ip_and_port = nodes_ip_and_port[i * send_num + j];
              senders.push_back(std::thread(send_to_datanode, j, k, shard_id, data, coding, true_shard_size, ip_and_port));
            }
            for (int j = 0; j < int(senders.size()); j++)
            {
              senders[j].join();
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
          if (status.ok())
          {
            std::cout << "connect coordinator,ok" << std::endl;
          }
          else
          {
            std::cout << "oooooH coordinator,fail!!!!" << std::endl;
          }
        }
      }
      catch (std::exception &e)
      {
        std::cout << "exception in encode_and_save" << std::endl;
        std::cout << e.what() << std::endl;
      }
    };
    try
    {
      std::thread my_thread(encode_and_save);
      my_thread.detach();
    }
    catch (std::exception &e)
    {
      std::cout << "exception" << std::endl;
      std::cout << e.what() << std::endl;
    }
    std::cout << "encode receive askDNhandling rpc!\n";

    return grpc::Status::OK;
  }

  grpc::Status ProxyImpl::decodeAndGetObject(
      grpc::ServerContext *context,
      const proxy_proto::ObjectAndPlacement *object_and_placement,
      proxy_proto::GetReply *response)
  {
    OppoProject::EncodeType encode_type = (OppoProject::EncodeType)object_and_placement->encode_type();
    std::string key = object_and_placement->key();
    bool big = object_and_placement->bigobject();
    int k = object_and_placement->k();
    int m = object_and_placement->m();
    int real_l = object_and_placement->real_l();
    int shard_size = object_and_placement->shard_size();
    int tail_shard_size = object_and_placement->tail_shard_size();
    int value_size_bytes = object_and_placement->valuesizebyte();
    std::string clientip = object_and_placement->clientip();
    int clientport = object_and_placement->clientport();
    std::vector<unsigned int> stripe_ids;

    for (int i = 0; i < object_and_placement->stripe_ids_size(); i++)
    {
      stripe_ids.push_back(object_and_placement->stripe_ids(i));
    }

    std::vector<std::pair<std::string, int>> nodes_ip_and_port;

    for (int i = 0; i < object_and_placement->datanodeip_size(); i++)
    {
      nodes_ip_and_port.push_back({object_and_placement->datanodeip(i), object_and_placement->datanodeport(i)});
    }

    auto decode_and_get = [this, big, key, k, m, real_l, shard_size, tail_shard_size, value_size_bytes,
                           clientip, clientport, stripe_ids, nodes_ip_and_port, encode_type]() mutable
    {
      if (big)
      {
        std::string value;
        for (int i = 0; i < int(stripe_ids.size()); i++)
        {
          auto shards_ptr = std::make_shared<std::vector<std::vector<char>>>();
          auto shards_idx_ptr = std::make_shared<std::vector<int>>();
          auto myLock_ptr = std::make_shared<std::mutex>();
          auto cv_ptr = std::make_shared<std::condition_variable>();
          int expect_block_number = (encode_type == Azure_LRC_1) ? (k + real_l - 1) : k;
          int all_expect_blocks = (encode_type == Azure_LRC_1) ? (k + m + real_l) : (k + m);
          int send_num = (encode_type == RS) ? (k + m) : (k + m + real_l + 1);

          std::vector<char *> v_data(k);
          std::vector<char *> v_coding(all_expect_blocks - k);
          char **data = v_data.data();
          char **coding = v_coding.data();

          auto getFromNode = [this, k, shards_ptr, shards_idx_ptr, myLock_ptr, cv_ptr](int expect_block_number, int stripe_id, int shard_idx, int x_shard_size, std::string ip, int port)
          {
            std::string shard_id = std::to_string(stripe_id * 1000 + shard_idx);
            std::vector<char> temp(x_shard_size);
            size_t temp_size;
            bool ret = GetFromMemcached(shard_id.c_str(), shard_id.size(), temp.data(), &temp_size, 0, x_shard_size, ip.c_str(), port);

            if (!ret)
            {
              std::cout << "getFromNode !ret" << std::endl;
              return;
            }
            myLock_ptr->lock();

            if (!check_received_block(k, expect_block_number, shards_idx_ptr, shards_ptr->size()))
            {
              shards_ptr->push_back(temp);
              shards_idx_ptr->push_back(shard_idx);
              if (check_received_block(k, expect_block_number, shards_idx_ptr, shards_ptr->size()))
              {
                cv_ptr->notify_all();
              }
              // 检查已有的块是否满足要求
            }
            myLock_ptr->unlock();
          };

          int true_shard_size;
          if ((i == int(stripe_ids.size() - 1)) && tail_shard_size != -1)
          {
            true_shard_size = tail_shard_size;
          }
          else
          {
            true_shard_size = shard_size;
          }

          std::vector<std::vector<char>> v_data_area(k, std::vector<char>(true_shard_size));
          std::vector<std::vector<char>> v_coding_area(all_expect_blocks - k, std::vector<char>(true_shard_size));
          for (int j = 0; j < k; j++)
          {
            data[j] = v_data_area[j].data();
          }
          for (int j = 0; j < all_expect_blocks - k; j++)
          {
            coding[j] = v_coding_area[j].data();
          }
          std::vector<std::thread> read_memcached_treads;

          for (int j = 0; j < all_expect_blocks; j++)
          {
            std::pair<std::string, int> &ip_and_port = nodes_ip_and_port[i * send_num + j];
            read_memcached_treads.push_back(std::thread(
                getFromNode, expect_block_number, stripe_ids[i], j, true_shard_size, ip_and_port.first, ip_and_port.second));
          }
          for (int j = 0; j < all_expect_blocks; j++)
          {
            read_memcached_treads[j].detach();
          }

          std::unique_lock<std::mutex> lck(*myLock_ptr);

          while (!check_received_block(k, expect_block_number, shards_idx_ptr, shards_ptr->size()))
          {
            cv_ptr->wait(lck);
          }
          for (int j = 0; j < int(shards_idx_ptr->size()); j++)
          {
            int idx = (*shards_idx_ptr)[j];
            if (idx < k)
            {
              memcpy(data[idx], (*shards_ptr)[j].data(), true_shard_size);
            }
            else
            {
              memcpy(coding[idx - k], (*shards_ptr)[j].data(), true_shard_size);
            }
          }

          auto erasures = std::make_shared<std::vector<int>>();
          for (int j = 0; j < all_expect_blocks; j++)
          {
            if (std::find(shards_idx_ptr->begin(), shards_idx_ptr->end(), j) == shards_idx_ptr->end())
            {
              erasures->push_back(j);
            }
          }
          erasures->push_back(-1);
          if (encode_type == RS || encode_type == OPPO_LRC)
          {
            decode(k, m, 0, data, coding, erasures, true_shard_size, encode_type);
          }
          else if (encode_type == Azure_LRC_1)
          {
            decode(k, m, real_l, data, coding, erasures, true_shard_size, encode_type);
          }
          else
          {
            std::cout << "proxy解码类型出错了友友" << std::endl;
          }

          for (int j = 0; j < k; j++)
          {
            value += std::string(data[j], true_shard_size);
          }
        }
        asio::error_code error;
        asio::ip::tcp::resolver resolver(io_context);
        asio::ip::tcp::resolver::results_type endpoints =
            resolver.resolve(clientip, std::to_string(clientport));

        asio::ip::tcp::socket sock_data(io_context);
        asio::connect(sock_data, endpoints);

        asio::write(sock_data, asio::buffer(key, key.size()), error);
        asio::write(sock_data, asio::buffer(value, value_size_bytes), error);
        sock_data.shutdown(asio::ip::tcp::socket::shutdown_send);
        sock_data.close();
      }
    };
    try
    {
      std::thread my_thread(decode_and_get);
      my_thread.detach();
    }
    catch (std::exception &e)
    {
      std::cout << "exception" << std::endl;
      std::cout << e.what() << std::endl;
    }

    return grpc::Status::OK;
  }

  bool
  ProxyImpl::init_coordinator()
  {
    std::string coordinator_ip_port = "0.0.0.0:55555";
    m_coordinator_stub =
        coordinator_proto::CoordinatorService::NewStub(grpc::CreateChannel(
            coordinator_ip_port, grpc::InsecureChannelCredentials()));
    return true;
  }

  grpc::Status ProxyImpl::mainRepair(
      grpc::ServerContext *context,
      const proxy_proto::mainRepairPlan *mainRepairPlan,
      proxy_proto::mainRepairReply *reply)
  {
    std::cout << "mainRepair" << std::endl;
    bool one_shard_fail = mainRepairPlan->one_shard_fail();
    std::vector<std::pair<std::pair<std::string, int>, int>> inner_az_shards_to_read;
    for (int i = 0; i < mainRepairPlan->inner_az_help_shards_ip_size(); i++) {
      inner_az_shards_to_read.push_back({
          {mainRepairPlan->inner_az_help_shards_ip(i), mainRepairPlan->inner_az_help_shards_port(i)},
          mainRepairPlan->inner_az_help_shards_idx(i)
        });
    }
    int k = mainRepairPlan->k();
    int g = mainRepairPlan->g();
    std::vector<std::pair<std::pair<std::string, int>, int>> new_locations_with_shard_idx;
    for (int i = 0; i < mainRepairPlan->new_location_ip_size(); i++) {
      new_locations_with_shard_idx.push_back({
          {mainRepairPlan->new_location_ip(i), mainRepairPlan->new_location_port(i)},
          mainRepairPlan->new_location_shard_idx(i)
        });
    }
    int self_az_id = mainRepairPlan->self_az_id();
    std::vector<int> help_azs_id;
    for (int i = 0; i < mainRepairPlan->help_azs_id_size(); i++) {
      help_azs_id.push_back(mainRepairPlan->help_azs_id(i));
    }
    bool if_partial_decoding = mainRepairPlan->if_partial_decoding();
    int stripe_id = mainRepairPlan->stripe_id();
    int shard_size = mainRepairPlan->shard_size();
    EncodeType encode_type = EncodeType(mainRepairPlan->encode_type());
    std::unordered_map<int, std::unordered_map<int, std::vector<char>>> data_or_parity;
    std::vector<std::thread> readers_inner_az;
    std::vector<std::thread> readers_other_az;

    if (one_shard_fail) {
      for (int i = 0; i < int(inner_az_shards_to_read.size()); i++) {
        readers_inner_az.push_back(std::thread([&, i](){
          std::string &ip = inner_az_shards_to_read[i].first.first;
          int port = inner_az_shards_to_read[i].first.second;
          int shard_idx = inner_az_shards_to_read[i].second;
          std::vector<char> buf(shard_size);
          std::string shard_id = std::to_string(stripe_id * 1000 + shard_idx);
          size_t temp_size;
          GetFromMemcached(shard_id.c_str(), shard_id.size(), buf.data(), &temp_size, 0, shard_size, ip.c_str(), port);
          repair_buffer_lock.lock();
          data_or_parity[self_az_id][shard_idx] = buf;
          repair_buffer_lock.unlock();
        }));
      }
      for (int i = 0; i < int(help_azs_id.size()); i++) {
        std::shared_ptr<asio::ip::tcp::socket> socket_ptr = std::make_shared<asio::ip::tcp::socket>(io_context);
        acceptor.accept(*socket_ptr);
        readers_other_az.push_back(std::thread([&, socket_ptr](){
          std::vector<unsigned char> int_buf(sizeof(int));
          asio::read(*socket_ptr, asio::buffer(int_buf, int_buf.size()));
          int az_id = OppoProject::bytes_to_int(int_buf);
          if (if_partial_decoding) {
            std::vector<char> buf(shard_size);
            asio::read(*socket_ptr, asio::buffer(buf, buf.size()));
            repair_buffer_lock.lock();
            data_or_parity[az_id][-1] = buf;
            repair_buffer_lock.unlock();
          } else {
            std::vector<unsigned char> int_buf(sizeof(int));
            asio::read(*socket_ptr, asio::buffer(int_buf, int_buf.size()));
            int num_of_shard = OppoProject::bytes_to_int(int_buf);
            for (int j = 0; j < num_of_shard; j++) {
              asio::read(*socket_ptr, asio::buffer(int_buf, int_buf.size()));
              int shard_idx = OppoProject::bytes_to_int(int_buf);
              std::vector<char> buf(shard_size);
              asio::read(*socket_ptr, asio::buffer(buf, buf.size()));
              repair_buffer_lock.lock();
              data_or_parity[az_id][shard_idx] = buf;
              repair_buffer_lock.unlock();
            }
          }
          socket_ptr->shutdown(asio::ip::tcp::socket::shutdown_receive);
          socket_ptr->close();
        }));
      }
      for (auto &th : readers_inner_az) {
        th.join();
      }
      for (auto &th : readers_other_az) {
        th.join();
      }
      std::vector<char> repaired_shard(shard_size);
      int failed_shard_idx = new_locations_with_shard_idx[0].second;
      if (encode_type == RS && (failed_shard_idx >= k && failed_shard_idx <= (k + g - 1))) {
        int g_row = failed_shard_idx - k;
        int *rs_matrix = reed_sol_vandermonde_coding_matrix(k, g, 8);
        std::vector<int> matrix(g * k, 0);
        memcpy(matrix.data(), rs_matrix, g * k * sizeof(int));
        free(rs_matrix);
        int *coefficients = matrix.data() + g_row * k;
        int count1 = data_or_parity[self_az_id].size();
        int count2 = 0;
        for (auto &p : data_or_parity) {
          if (p.first != self_az_id) {
            count2 += p.second.size();
          }
        }
        std::vector<char *> v_data(count1 + count2);
        std::vector<char *> v_coding(1);
        char **data = (char **)v_data.data();
        char **coding = (char **)v_coding.data();
        std::vector<int> new_matrix(count1 + count2, 1);
        int idx = 0;
        for (auto &q : data_or_parity[self_az_id]) {
          data[idx] = q.second.data();
          std::cout << new_matrix[idx] << " ";
          new_matrix[idx] = coefficients[q.first];
          idx++;
        }
        for (auto &p : data_or_parity) {
          if (p.first != self_az_id) {
            for (auto &q : p.second) {
              data[idx] = q.second.data();
              if (!if_partial_decoding) {
                std::cout << new_matrix[idx] << " ";
                new_matrix[idx] = coefficients[q.first];
              }
              idx++;
            }
          }
        }
        std::cout << std::endl;
        coding[0] = repaired_shard.data();
        jerasure_matrix_encode(count1 + count2, 1, 8, new_matrix.data(), data, coding, shard_size);
      } else {
        int count = 0;
        for (auto &p : data_or_parity) {
          count += p.second.size();
        }
        std::vector<char *> v_data(count);
        std::vector<char *> v_coding(1);
        char **data = (char **)v_data.data();
        char **coding = (char **)v_coding.data();
        int idx = 0;
        for (auto &p : data_or_parity) {
          for (auto &q : p.second) {
            data[idx++] = q.second.data();
          }
        }
        coding[0] = repaired_shard.data();
        std::vector<int> new_matrix(1 * count, 1);
        jerasure_matrix_encode(count, 1, 8, new_matrix.data(), data, coding, shard_size);
      }
      std::string &new_node_ip = new_locations_with_shard_idx[0].first.first;
      int new_node_port = new_locations_with_shard_idx[0].first.second;
      std::string failed_shard_id = std::to_string(stripe_id * 1000 + failed_shard_idx);
      SetToMemcached(failed_shard_id.c_str(), failed_shard_id.size(), repaired_shard.data(), shard_size, new_node_ip.c_str(), new_node_port);
    } else {

    }
    std::cout << "main repair done" << std::endl;
    return grpc::Status::OK;
  }

  grpc::Status ProxyImpl::helpRepair(
      grpc::ServerContext *context,
      const proxy_proto::helpRepairPlan *helpRepairPlan,
      proxy_proto::helpRepairReply *reply)
  {
    std::cout << "helpRepair" << std::endl;
    bool one_shard_fail = helpRepairPlan->one_shard_fail();
    std::vector<std::pair<std::pair<std::string, int>, int>> inner_az_shards_to_read;
    for (int i = 0; i < helpRepairPlan->inner_az_help_shards_ip_size(); i++) {
      inner_az_shards_to_read.push_back({
          {helpRepairPlan->inner_az_help_shards_ip(i), helpRepairPlan->inner_az_help_shards_port(i)},
          helpRepairPlan->inner_az_help_shards_idx(i)
        });
    }
    int k = helpRepairPlan->k();
    int g = helpRepairPlan->g();
    int self_az_id = helpRepairPlan->self_az_id();
    bool if_partial_decoding = helpRepairPlan->if_partial_decoding();
    int stripe_id = helpRepairPlan->stripe_id();
    int shard_size = helpRepairPlan->shard_size();
    EncodeType encode_type = EncodeType(helpRepairPlan->encode_type());
    std::string main_proxy_ip = helpRepairPlan->main_proxy_ip();
    int main_proxy_port = helpRepairPlan->main_proxy_port();
    int failed_shard_idx = helpRepairPlan->failed_shard_idx();
    std::unordered_map<int, std::unordered_map<int, std::vector<char>>> data_or_parity;
    std::vector<std::thread> readers_inner_az;

    if (one_shard_fail) {
      for (int i = 0; i < int(inner_az_shards_to_read.size()); i++) {
        readers_inner_az.push_back(std::thread([&, i](){
          std::string &ip = inner_az_shards_to_read[i].first.first;
          int port = inner_az_shards_to_read[i].first.second;
          int shard_idx = inner_az_shards_to_read[i].second;
          std::vector<char> buf(shard_size);
          std::string shard_id = std::to_string(stripe_id * 1000 + shard_idx);
          size_t temp_size;
          GetFromMemcached(shard_id.c_str(), shard_id.size(), buf.data(), &temp_size, 0, shard_size, ip.c_str(), port);
          repair_buffer_lock.lock();
          data_or_parity[self_az_id][shard_idx] = buf;
          repair_buffer_lock.unlock();
        }));
      }
      for (auto &th : readers_inner_az) {
        th.join();
      }
      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(main_proxy_ip, std::to_string(main_proxy_port));
      asio::ip::tcp::socket socket(io_context);
      asio::connect(socket, endpoint);
      std::vector<unsigned char> int_buf_self_az_id = OppoProject::int_to_bytes(self_az_id);
      asio::write(socket, asio::buffer(int_buf_self_az_id, int_buf_self_az_id.size()));
      if (if_partial_decoding) {
        std::vector<char> merge_result(shard_size, 1);
        std::vector<int> matrix;
        int *coefficients = NULL;
        if (encode_type == RS && (failed_shard_idx >= k && failed_shard_idx <= (k + g - 1))) {
          int g_row = failed_shard_idx - k;
          std::cout << "k: " << k << " g: " << g << " failed_shard_idx: " << failed_shard_idx << std::endl;
          std::cout << "fuck" << std::endl;
          int *rs_matrix = reed_sol_vandermonde_coding_matrix(k, g, 8);
          matrix.resize(g * k);
          memcpy(matrix.data(), rs_matrix, g * k * sizeof(int));
          free(rs_matrix);
          for (int i = 0; i < g; i++) {
            for (int j = 0; j < k; j++) {
              std::cout << matrix[i * k + j] << " ";
            }
            std::cout << std::endl;
          }
          coefficients = matrix.data() + g_row * k;
        }
        int count = 0;
        for (auto &p : data_or_parity) {
          count += p.second.size();
        }
        std::vector<char *> v_data(count);
        std::vector<char *> v_coding(1);
        char **data = (char **)v_data.data();
        char **coding = (char **)v_coding.data();
        std::vector<int> new_matrix(1 * count, 1);
        int idx = 0;
        for (auto &p : data_or_parity) {
          for (auto &q : p.second) {
            data[idx] = q.second.data();
            if (encode_type == RS && (failed_shard_idx >= k && failed_shard_idx <= (k + g - 1))) {
              new_matrix[idx] = coefficients[q.first];
            }
            idx++;
          }
        }
        coding[0] = merge_result.data();
        jerasure_matrix_encode(count, 1, 8, new_matrix.data(), data, coding, shard_size);
        asio::write(socket, asio::buffer(merge_result, merge_result.size()));
      } else {
        std::vector<unsigned char> int_buf_num_of_shards = OppoProject::int_to_bytes(inner_az_shards_to_read.size());
        asio::write(socket, asio::buffer(int_buf_num_of_shards, int_buf_num_of_shards.size()));
        for (auto &p : data_or_parity) {
          for (auto &q : p.second) {
            std::vector<unsigned char> int_buf_shard_idx = OppoProject::int_to_bytes(q.first);
            asio::write(socket, asio::buffer(int_buf_shard_idx, int_buf_shard_idx.size()));
            asio::write(socket, asio::buffer(q.second, q.second.size()));
          }
        }
      }
      socket.shutdown(asio::ip::tcp::socket::shutdown_send);
      socket.close();
    }
    std::cout << "helpRepair done" << std::endl;
    return grpc::Status::OK;
  }
} // namespace OppoProject
