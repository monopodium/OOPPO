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
      std::cout << "SetToMemcached"
                << " " << std::string(ip) << " " << port << std::endl;
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
      std::cout << "Value to Set is :"
                << " " << value << std::endl;
      std::vector<char> finish(1);
      asio::read(socket, asio::buffer(finish, finish.size()));

      asio::error_code ignore_ec;
      socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
      socket.close(ignore_ec);
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
      std::cerr << "proxy GetFromMemcached"
                << " " << std::string(ip) << " " << port << std::endl;
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
      
      // std::cerr << "getfromMemcached start read" << std::endl;
      // std::cerr << "getfromMemcached key " << key << std::endl;
      // std::cerr << "getfromMemcached offset " << offset << std::endl;
      // std::cerr << "getfromMemcached len" << lenth << std::endl;
      asio::read(socket, asio::buffer(value_from_datanode, value_from_datanode.size()));
      // std::cerr << "getfromMemcached finish read" << std::endl;
      // std::cerr << "value_from_datanode.data() is " << value_from_datanode.data() << std::endl;

      asio::error_code ignore_ec;
      socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
      socket.close(ignore_ec);

      memcpy(value, value_from_datanode.data(), lenth);
      // std::cerr << "value_from_datanode is:" << value << std::endl;
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
          asio::error_code ignore_ec;
          socket_data.shutdown(asio::ip::tcp::socket::shutdown_receive, ignore_ec);
          socket_data.close(ignore_ec);

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

  grpc::Status ProxyImpl::WriteBufferAndEncode(
      grpc::ServerContext *context,
      const proxy_proto::ObjectAndPlacement *object_and_placement,
      proxy_proto::SetReply *response)
  {
    /*metadata record*/
    //std::unique_lock<std::mutex> lck(proxybuf_lock);
    std::string key = object_and_placement->key();
    bool big = object_and_placement->bigobject();
    int value_size_bytes = object_and_placement->valuesizebyte();
    int k = object_and_placement->k();
    int m = object_and_placement->m();
    int real_l = object_and_placement->real_l();
    int shard_size = object_and_placement->shard_size();
    int tail_shard_size = object_and_placement->tail_shard_size();
    OppoProject::EncodeType encode_type = (OppoProject::EncodeType)object_and_placement->encode_type();
    int buf_idx = object_and_placement->writebufferindex();
    /*init prxox buffer infomation*/
    static int init_flag = true;
    if (init_flag)
    {
      std::vector<std::vector<char>> tmp_buf(k, std::vector<char>(shard_size));
      std::vector<int> tmp_buf_off(k, 0);
      proxy_buf = tmp_buf;
      buf_offset = tmp_buf_off;
      init_flag = false;
    }
    if (buf_idx == -1)
    {
      /*encode old buffer*/
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
      auto encode_buf_and_save = [this, big, key, value_size_bytes, k, m, real_l, shard_size, tail_shard_size, stripe_ids, nodes_ip_and_port, encode_type]() mutable
      {
        try
        {
          /*lambda expression for data_send*/
          // sem_init(&sem,0,0);
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
          // encoding
          std::vector<char *> v_data(k);
          std::vector<char *> v_coding(m + real_l + 1);
          char **data = (char **)v_data.data();
          char **coding = (char **)v_coding.data();
          int true_shard_size = shard_size;
          std::vector<std::vector<char>> v_coding_area(m + real_l + 1, std::vector<char>(true_shard_size));
          std::vector<std::vector<char>> v_data_area(k, std::vector<char>(true_shard_size));
          {// lock the proxy_buf
            std::lock_guard<std::mutex> lock(proxybuf_lock);
            for (int j = 0; j < k; j++)
            {
              data[j] = v_data_area[j].data();
              std::copy(proxy_buf[j].begin(), proxy_buf[j].end(), data[j]);
              std::cout << "proxy[" << j << "] is " << std::string(proxy_buf[j].begin(), proxy_buf[j].end()) << std::endl;
            }
          }
          sem_post(&sem);
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
          // set to memcached
          std::vector<std::thread> senders;
          for (int j = 0; j < send_num; j++)
          {
            if(j<k)
              std::cout << "data["<< j <<"] is :" << data[j] << std::endl;
            std::string shard_id = std::to_string(stripe_ids[0] * 1000 + j);
            std::pair<std::string, int> ip_and_port = nodes_ip_and_port[0 * send_num + j];
            senders.push_back(std::thread(send_to_datanode, j, k, shard_id, data, coding, true_shard_size, ip_and_port));
            std::cout << "shard_id : " << shard_id << " port: " << ip_and_port.second <<std::endl;
          }
          for (int j = 0; j < int(senders.size()); j++)
          {
            senders[j].join();
          }
        }
        catch (std::exception &e)
        {
          std::cout << "exception in encode_buf_and_save" << std::endl;
          std::cout << e.what() << std::endl;
        }
      };
      try
      {
        std::thread my_thread(encode_buf_and_save);
        my_thread.detach();
      }
      catch (std::exception &e)
      {
        std::cout << "exception" << std::endl;
        std::cout << e.what() << std::endl;
      }
      std::cout << "encode receive askDNhandling rpc!\n";
      /*reinit the proxy buffer(to lock)*/
      // sleep(1);
      sem_wait(&sem);
      std::cout << "reinit proxy_buf & buf_offset" << std::endl;
      proxy_buf = std::vector<std::vector<char>>(k, std::vector<char>(shard_size));
      memset(&buf_offset[0], 0, sizeof(buf_offset[0]) * buf_offset.size());
      //sem_destroy(&sem);
    }
    else
    {
      /*write data into proxy buffer*/
      auto write_buffer = [this, key, value_size_bytes, k, m, shard_size,
                           buf_idx]() mutable
      {
        asio::ip::tcp::socket socket_data(io_context);
        acceptor.accept(socket_data);
        asio::error_code error;

        /*read data*/
        std::vector<char> buf_key(key.size());
        size_t len = socket_data.read_some(asio::buffer(buf_key, key.size()), error);
        if (error == asio::error::eof)
        {
          std::cout << "error == asio::error::eof" << std::endl;
        }
        else if (error)
        {
          throw asio::system_error(error);
        }
        /*check key*/
        int flag = 1;
        for (int i = 0; i < key.size(); i++)
        {
          if (key[i] != buf_key[i])
          {
            flag = 0;
            std::cout << "write buffer key wrong!" << std::endl;
          }
        }
        /*write value to buffer*/
        if (flag)
        {
          std::vector<char> buf(value_size_bytes);
          len = socket_data.read_some(asio::buffer(buf, value_size_bytes), error);
          std::copy(buf.begin(),buf.end(),proxy_buf[buf_idx].begin()+buf_offset[buf_idx]);
          // std::cout << "proxy_ip_port is :" << proxy_ip_port << std::endl; 
          // std::cout << "obj key is :" << key << std::endl;
          // std::cout << "buffer write:" << len << " bytes" << std::endl;
          // std::cout << "buffer idx:" << buf_idx << std::endl;
          // std::cout << "buffer offset:" << buf_offset[buf_idx] << " bytes" << std::endl;
          std::string s(proxy_buf[buf_idx].begin()+buf_offset[buf_idx],proxy_buf[buf_idx].begin()+buf_offset[buf_idx]+len);
          // std::cout << "proxy_buf write:" << s << std::endl;
          buf_offset[buf_idx] += value_size_bytes;
          // std::cout << "buffer offset after update:" << buf_offset[buf_idx] << " bytes" << std::endl;
        }
        asio::error_code ignore_ec;
        socket_data.shutdown(asio::ip::tcp::socket::shutdown_receive, ignore_ec);
        socket_data.close(ignore_ec);
        /*commitAbort*/
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
      };
      try
      {
        std::thread my_thread(write_buffer);
        my_thread.detach();
      }
      catch (std::exception &e)
      {
        std::cout << "exception in write buffer" << std::endl;
        std::cout << e.what() << std::endl;
      }
      std::cout << "receive askDNhandling rpc!\n";
    }

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
    int obj_offset = object_and_placement->offset();
    int shard_idx = object_and_placement->shard_idx();
    int obj_size = object_and_placement->obj_size();
    std::vector<unsigned int> stripe_ids;

    // std::cerr << "stripe_ids_size is:  "<< object_and_placement->stripe_ids_size() << std::endl;
    // std::cerr << "shard_idx:  "<< object_and_placement->shard_idx() << std::endl;

    for (int i = 0; i < object_and_placement->stripe_ids_size(); i++)
    {
      stripe_ids.push_back(object_and_placement->stripe_ids(i));
    }

    std::vector<std::pair<std::string, int>> nodes_ip_and_port;

    for (int i = 0; i < object_and_placement->datanodeip_size(); i++)
    {
      nodes_ip_and_port.push_back({object_and_placement->datanodeip(i), object_and_placement->datanodeport(i)});
    }

    auto decode_and_get = [this, big, key, k, m, real_l, shard_size, tail_shard_size, value_size_bytes, obj_offset, shard_idx,obj_size,
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
          int send_num;
          if (encode_type == RS)
          {
            send_num = k + m;
          }
          else if (encode_type == Azure_LRC_1)
          {
            // m = g for lrc
            send_num = k + m + real_l + 1;
          }
          else if (encode_type == OPPO_LRC)
          {
            send_num = k + m + real_l;
          }

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
            if (!decode(k, m, real_l, data, coding, erasures, true_shard_size, encode_type))
            {
              std::cout << "cannot decode!" << std::endl;
            }
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
        asio::error_code ignore_ec;
        sock_data.shutdown(asio::ip::tcp::socket::shutdown_send, ignore_ec);
        sock_data.close(ignore_ec);
      }else{
        std::cerr << "proxy decode_and_get" << std::endl;
        std::string value;
        auto shards_ptr = std::make_shared<std::vector<std::vector<char>>>();
        auto shards_idx_ptr = std::make_shared<std::vector<int>>();
        auto myLock_ptr = std::make_shared<std::mutex>();
        auto cv_ptr = std::make_shared<std::condition_variable>();

        std::vector<char *> v_data(1);
        char **data = v_data.data();
        std::vector<char> v_data_area(obj_size);
        data[0] = v_data_area.data();
        auto getFromNode = [this, k, shards_ptr, shards_idx_ptr, myLock_ptr, cv_ptr,obj_offset,obj_size](int expect_block_number, int stripe_id, int shard_idx, int x_shard_size, std::string ip, int port)
        {
          // std::cerr << "proxy get_from_node" << std::endl;
          // std::cerr << "stripe_id: " << stripe_id <<std::endl;
          // std::cerr << "shard_idx: " << shard_idx << std::endl;
          // std::cerr << "obj_size: " << obj_size << std::endl;
          std::string shard_id = std::to_string(stripe_id * 1000 + shard_idx);
          // std::cerr << "proxy shard_id is " << shard_id <<std::endl;
          std::vector<char> temp(obj_size);
          size_t temp_size;
          // std::cerr << "get_from_memcached begin" << std::endl;
          bool ret = GetFromMemcached(shard_id.c_str(), shard_id.size(), temp.data(), &temp_size, obj_offset, obj_size, ip.c_str(), port);
          // std::cerr << "get_from_memcached finish" << std::endl;
          if (!ret)
          {
            std::cout << "getFromNode !ret" << std::endl;
            return;
          }
          // std::cerr << "get_from_memcached is " << temp.data() << std::endl;
          myLock_ptr->lock();

          if (!check_received_block(k, 1, shards_idx_ptr, shards_ptr->size()))
          {
            shards_ptr->push_back(temp);
            shards_idx_ptr->push_back(shard_idx);
            if (check_received_block(k, 1, shards_idx_ptr, shards_ptr->size()))
            {
              cv_ptr->notify_all();
            }
            // 检查已有的块是否满足要求
          }
          myLock_ptr->unlock();
        };

        int true_shard_size = shard_size;
        std::pair<std::string, int> &ip_and_port = nodes_ip_and_port[0];
        // std::cerr << "stripeId: " << stripe_ids[0] << std::endl; 
        // std::cerr << "shard_idx: " << shard_idx << std::endl; 
        try
        {
          // std::cerr << "get_from_node_thread start" << std::endl;
          std::thread read_memcached_tread(
            getFromNode, 1, stripe_ids[0], shard_idx, true_shard_size, ip_and_port.first, ip_and_port.second);
          read_memcached_tread.detach();
          // std::cerr << "get_from_node_thread detach" << std::endl;
        }
        catch (std::exception &e)
        {
          std::cout << "exception" << std::endl;
          std::cout << e.what() << std::endl;
        }
        std::unique_lock<std::mutex> lck(*myLock_ptr);
        while (!check_received_block(k, 1, shards_idx_ptr, shards_ptr->size()))
        {
          cv_ptr->wait(lck);
        }
        // std::cerr << "shards_ptr size(): " << shards_ptr->size() << std::endl;
        // std::cerr << "shards_ptr[0] size(): " << (*shards_ptr)[0].size() << std::endl;
        // std::cerr << "obj_size is "<< obj_size << std::endl;
        // std::cerr << "before decode_and_get's memcpy" << std::endl;
        memcpy(data[0], (*shards_ptr)[0].data(), obj_size);
        // std::cerr << "after decode_and_get's memcpy" << std::endl;
        value += std::string(data[0], obj_size);
        // std::cerr<< "length of value " << value.size() << std::endl;
        // std::cerr<< "value is" << value.data() << std::endl;
        asio::error_code error;
        asio::ip::tcp::resolver resolver(io_context);
        asio::ip::tcp::resolver::results_type endpoints =
            resolver.resolve(clientip, std::to_string(clientport));

        asio::ip::tcp::socket sock_data(io_context);
        asio::connect(sock_data, endpoints);

        asio::write(sock_data, asio::buffer(key, key.size()), error);
        asio::write(sock_data, asio::buffer(value, value_size_bytes), error);
        asio::error_code ignore_ec;
        sock_data.shutdown(asio::ip::tcp::socket::shutdown_send, ignore_ec);
        sock_data.close(ignore_ec);
      }
    };
    try
    {
      // std::cerr << "decode_and_get_thread start" << std::endl;
      std::thread my_thread(decode_and_get);
      my_thread.detach();
      // std::cerr << "decode_and_get_thread detach" << std::endl;
    }
    catch (std::exception &e)
    {
      std::cout << "exception" << std::endl;
      std::cout << e.what() << std::endl;
    }

    return grpc::Status::OK;
  }

  grpc::Status ProxyImpl::getObjectFromBuffer(
      grpc::ServerContext *context,
      const proxy_proto::ObjectAndPlacement *object_and_placement,
      proxy_proto::GetReply *response)
  {
    // std::cout << "start get object from buffer!" << std::endl;
    /*find the obj*/
    std::string key = object_and_placement->key();
    int obj_offset = object_and_placement->offset();
    int shard_idx = object_and_placement->shard_idx();
    int obj_size = object_and_placement->obj_size();
    std::string clientip = object_and_placement->clientip();
    int clientport = object_and_placement->clientport();
    std::vector<char> v_data_area(obj_size);
    char *data = v_data_area.data();
    // std::cout << "read from buffer key: " << key << std::endl;
    // std::cout << "read from buffer shard_idx: " << shard_idx << std::endl;
    // std::cout << "read from buffer offset: " << obj_offset << std::endl;
    // std::cout << "read from buffer size: " << obj_size << std::endl;
    std::copy(proxy_buf[shard_idx].begin()+obj_offset, proxy_buf[shard_idx].begin()+obj_offset+obj_size, data);
    // std::cout << "read from buffer: " << data << std::endl;
    asio::error_code error;
    asio::ip::tcp::resolver resolver(io_context);
    asio::ip::tcp::resolver::results_type endpoints =
        resolver.resolve(clientip, std::to_string(clientport));
    asio::ip::tcp::socket sock_data(io_context);
    asio::connect(sock_data, endpoints);
    asio::write(sock_data, asio::buffer(key, key.size()), error);
    asio::write(sock_data, asio::buffer(data, obj_size), error);
    asio::error_code ignore_ec;
    sock_data.shutdown(asio::ip::tcp::socket::shutdown_send, ignore_ec);
    sock_data.close(ignore_ec);
    return grpc::Status::OK;
  }

  bool
  ProxyImpl::init_coordinator()
  {
    std::string coordinator_ip_port = coordinator_ip + ":55555";
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
    for (int i = 0; i < mainRepairPlan->inner_az_help_shards_ip_size(); i++)
    {
      inner_az_shards_to_read.push_back({{mainRepairPlan->inner_az_help_shards_ip(i), mainRepairPlan->inner_az_help_shards_port(i)},
                                         mainRepairPlan->inner_az_help_shards_idx(i)});
    }
    int k = mainRepairPlan->k();
    int g = mainRepairPlan->g();
    int real_l = mainRepairPlan->real_l();
    std::vector<std::pair<std::pair<std::string, int>, int>> new_locations_with_shard_idx;
    for (int i = 0; i < mainRepairPlan->new_location_ip_size(); i++)
    {
      new_locations_with_shard_idx.push_back({{mainRepairPlan->new_location_ip(i), mainRepairPlan->new_location_port(i)},
                                              mainRepairPlan->new_location_shard_idx(i)});
    }
    int self_az_id = mainRepairPlan->self_az_id();
    bool if_partial_decoding = mainRepairPlan->if_partial_decoding();
    std::vector<int> help_azs_id;
    std::unordered_map<int, bool> merge;
    for (int i = 0; i < mainRepairPlan->help_azs_id_size(); i++)
    {
      help_azs_id.push_back(mainRepairPlan->help_azs_id(i));
    }
    if (if_partial_decoding && one_shard_fail == false)
    {
      for (int i = 0; i < mainRepairPlan->merge_size(); i++)
      {
        merge[help_azs_id[i]] = mainRepairPlan->merge(i);
      }
    }
    int stripe_id = mainRepairPlan->stripe_id();
    int shard_size = mainRepairPlan->shard_size();
    EncodeType encode_type = EncodeType(mainRepairPlan->encode_type());
    std::vector<int> all_failed_shards_idx;
    for (int i = 0; i < mainRepairPlan->all_failed_shards_idx_size(); i++)
    {
      all_failed_shards_idx.push_back(mainRepairPlan->all_failed_shards_idx(i));
    }
    partial_helper data_or_parity;
    std::vector<std::thread> readers_inner_az;
    std::vector<std::thread> readers_other_az;

    if (one_shard_fail)
    {
      for (int i = 0; i < int(inner_az_shards_to_read.size()); i++)
      {
        readers_inner_az.push_back(std::thread([&, i]()
                                               {
          std::string &ip = inner_az_shards_to_read[i].first.first;
          int port = inner_az_shards_to_read[i].first.second;
          int shard_idx = inner_az_shards_to_read[i].second;
          std::vector<char> buf(shard_size);
          std::string shard_id = std::to_string(stripe_id * 1000 + shard_idx);
          size_t temp_size;
          GetFromMemcached(shard_id.c_str(), shard_id.size(), buf.data(), &temp_size, 0, shard_size, ip.c_str(), port);
          repair_buffer_lock.lock();
          data_or_parity[self_az_id][shard_idx] = buf;
          repair_buffer_lock.unlock(); }));
      }
      for (int i = 0; i < int(help_azs_id.size()); i++)
      {
        std::shared_ptr<asio::ip::tcp::socket> socket_ptr = std::make_shared<asio::ip::tcp::socket>(io_context);
        acceptor.accept(*socket_ptr);
        readers_other_az.push_back(std::thread([&, socket_ptr]()
                                               {
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
          asio::error_code ignore_ec;
          socket_ptr->shutdown(asio::ip::tcp::socket::shutdown_receive, ignore_ec);
          socket_ptr->close(ignore_ec); }));
      }
      for (auto &th : readers_inner_az)
      {
        th.join();
      }
      for (auto &th : readers_other_az)
      {
        th.join();
      }
      std::vector<char> repaired_shard(shard_size);
      int failed_shard_idx = new_locations_with_shard_idx[0].second;
      if (encode_type == RS && (failed_shard_idx >= k && failed_shard_idx <= (k + g - 1)))
      {
        int g_row = failed_shard_idx - k;
        int *rs_matrix = reed_sol_vandermonde_coding_matrix(k, g, 8);
        std::vector<int> matrix(g * k, 0);
        memcpy(matrix.data(), rs_matrix, g * k * sizeof(int));
        free(rs_matrix);
        int *coefficients = matrix.data() + g_row * k;
        int count1 = data_or_parity[self_az_id].size();
        int count2 = 0;
        for (auto &p : data_or_parity)
        {
          if (p.first != self_az_id)
          {
            count2 += p.second.size();
          }
        }
        std::vector<char *> v_data(count1 + count2);
        std::vector<char *> v_coding(1);
        char **data = (char **)v_data.data();
        char **coding = (char **)v_coding.data();
        std::vector<int> new_matrix(count1 + count2, 1);
        int idx = 0;
        for (auto &q : data_or_parity[self_az_id])
        {
          data[idx] = q.second.data();
          std::cout << new_matrix[idx] << " ";
          new_matrix[idx] = coefficients[q.first];
          idx++;
        }
        for (auto &p : data_or_parity)
        {
          if (p.first != self_az_id)
          {
            for (auto &q : p.second)
            {
              data[idx] = q.second.data();
              if (!if_partial_decoding)
              {
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
      }
      else
      {
        int count = 0;
        for (auto &p : data_or_parity)
        {
          count += p.second.size();
        }
        std::vector<char *> v_data(count);
        std::vector<char *> v_coding(1);
        char **data = (char **)v_data.data();
        char **coding = (char **)v_coding.data();
        int idx = 0;
        for (auto &p : data_or_parity)
        {
          for (auto &q : p.second)
          {
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
    }
    else
    {
      std::vector<int> failed_data_and_parity;
      int all_shard_number = (encode_type == Azure_LRC_1) ? (k + g + real_l) : (k + g);

      for (int i = 0; i < int(all_failed_shards_idx.size()); i++)
      {
        if (all_failed_shards_idx[i] < all_shard_number)
        {
          failed_data_and_parity.push_back(all_failed_shards_idx[i]);
        }
      }
      std::set<int> func_idx;
      if (if_partial_decoding)
      {
        for (int i = 0; i < int(failed_data_and_parity.size()); i++)
        {
          if (failed_data_and_parity[i] >= k && failed_data_and_parity[i] < all_shard_number)
          {
            func_idx.insert(failed_data_and_parity[i]);
          }
        }
        for (int i = k; i < all_shard_number; i++)
        {
          if (func_idx.size() == failed_data_and_parity.size())
          {
            break;
          }
          func_idx.insert(i);
        }
      }
      for (int i = 0; i < int(inner_az_shards_to_read.size()); i++)
      {
        readers_inner_az.push_back(std::thread([&, i]()
                                               {
            std::string &ip = inner_az_shards_to_read[i].first.first;
            int port = inner_az_shards_to_read[i].first.second;
            int shard_idx = inner_az_shards_to_read[i].second;
            std::vector<char> buf(shard_size);
            std::string shard_id = std::to_string(stripe_id * 1000 + shard_idx);
            size_t temp_size;
            GetFromMemcached(shard_id.c_str(), shard_id.size(), buf.data(), &temp_size, 0, shard_size, ip.c_str(), port);
            repair_buffer_lock.lock();
            data_or_parity[self_az_id][shard_idx] = buf;
            repair_buffer_lock.unlock(); }));
      }
      for (int i = 0; i < int(help_azs_id.size()); i++)
      {
        std::shared_ptr<asio::ip::tcp::socket> socket_ptr = std::make_shared<asio::ip::tcp::socket>(io_context);
        acceptor.accept(*socket_ptr);
        readers_other_az.push_back(std::thread([&, socket_ptr]()
                                               {
            std::vector<unsigned char> int_buf(sizeof(int));
            asio::read(*socket_ptr, asio::buffer(int_buf, int_buf.size()));
            int az_id = OppoProject::bytes_to_int(int_buf);
            asio::read(*socket_ptr, asio::buffer(int_buf, int_buf.size()));
            int num_of_shard = OppoProject::bytes_to_int(int_buf);
            for (int j = 0; j < num_of_shard; j++) {
              asio::read(*socket_ptr, asio::buffer(int_buf, int_buf.size()));
              if (if_partial_decoding && merge[az_id]) {
                int temp_func_idx = OppoProject::bytes_to_int(int_buf);
                std::vector<char> buf(shard_size);
                asio::read(*socket_ptr, asio::buffer(buf, buf.size()));
                repair_buffer_lock.lock();
                data_or_parity[az_id][temp_func_idx] = buf;
                repair_buffer_lock.unlock();
              } else {
                int shard_idx = OppoProject::bytes_to_int(int_buf);
                std::vector<char> buf(shard_size);
                asio::read(*socket_ptr, asio::buffer(buf, buf.size()));
                repair_buffer_lock.lock();
                data_or_parity[az_id][shard_idx] = buf;
                repair_buffer_lock.unlock();
              }
            }
            asio::error_code ignore_ec;
            socket_ptr->shutdown(asio::ip::tcp::socket::shutdown_receive, ignore_ec);
            socket_ptr->close(ignore_ec); }));
      }
      for (auto &th : readers_inner_az)
      {
        th.join();
      }
      for (auto &th : readers_other_az)
      {
        th.join();
      }
      if (if_partial_decoding)
      {
        for (auto &p : merge)
        {
          std::cout << p.second << " && ";
        }
        std::cout << std::endl;
        std::vector<int> matrix;
        if (encode_type == Azure_LRC_1)
        {
          matrix.resize((g + real_l) * k);
          lrc_make_matrix(k, g, real_l, matrix.data());
        }
        else
        {
          int *rs_matrix = reed_sol_vandermonde_coding_matrix(k, g, 8);
          matrix.resize(g * k);
          memcpy(matrix.data(), rs_matrix, g * k * sizeof(int));
          free(rs_matrix);
        }

        for (auto &p : data_or_parity)
        {
          if (merge[p.first] == false || p.first == self_az_id)
          {
            std::vector<std::pair<int, std::vector<char>>> saved_merge;
            std::vector<char> merge_result(shard_size);
            int count = p.second.size();
            std::vector<char *> v_data(count);
            std::vector<char *> v_coding(1);
            char **data = (char **)v_data.data();
            char **coding = (char **)v_coding.data();
            for (auto &t : func_idx)
            {
              std::vector<int> new_matrix(1 * count, 1);
              int real_idx = t - k;
              int *coff = &(matrix[real_idx * k]);
              int idx = 0;
              for (auto &q : p.second)
              {
                if (q.first < k)
                {
                  new_matrix[idx] = coff[q.first];
                }
                else
                {
                  new_matrix[idx] = (q.first == t);
                }
                data[idx] = q.second.data();
                idx++;
              }
              int sum = 0;
              for (auto &num : new_matrix)
              {
                sum += num;
              }
              coding[0] = merge_result.data();
              jerasure_matrix_encode(count, 1, 8, new_matrix.data(), data, coding, shard_size);
              if (sum == 0)
              {
                std::vector<char> temp(shard_size, 0);
                merge_result = temp;
              }
              saved_merge.push_back({t, merge_result});
            }
            p.second.clear();
            for (auto &q : saved_merge)
            {
              p.second[q.first] = q.second;
            }
          }
        }
        std::unordered_map<int, std::vector<char>> right;
        for (auto &p : func_idx)
        {
          int count = data_or_parity.size();
          std::vector<char *> v_data(count);
          std::vector<char *> v_coding(1);
          char **data = (char **)v_data.data();
          char **coding = (char **)v_coding.data();
          std::vector<char> temp(shard_size);
          int idx = 0;
          for (auto &q : data_or_parity)
          {
            data[idx++] = q.second[p].data();
          }
          coding[0] = temp.data();
          std::vector<int> new_matrix(1 * count, 1);
          jerasure_matrix_encode(count, 1, 8, new_matrix.data(), data, coding, shard_size);
          right[p] = temp;
        }
        std::vector<int> left;
        std::sort(failed_data_and_parity.begin(), failed_data_and_parity.end());
        for (auto &p : failed_data_and_parity)
        {
          std::cout << p << " ";
        }
        std::cout << std::endl;
        for (auto &p : func_idx)
        {
          int real_idx = p - k;
          int *coff = &(matrix[real_idx * k]);
          for (int i = 0; i < int(failed_data_and_parity.size()); i++)
          {
            if (failed_data_and_parity[i] < k)
            {
              left.push_back(coff[failed_data_and_parity[i]]);
            }
            else
            {
              left.push_back(failed_data_and_parity[i] == p);
            }
          }
        }
        std::vector<int> invert(left.size());
        jerasure_invert_matrix(left.data(), invert.data(), failed_data_and_parity.size(), 8);
        std::vector<char *> v_data(failed_data_and_parity.size());
        std::vector<char *> v_coding(failed_data_and_parity.size());
        char **data = (char **)v_data.data();
        char **coding = (char **)v_coding.data();
        std::vector<std::vector<char>> repaired_shards(failed_data_and_parity.size(), std::vector<char>(shard_size));
        int idx = 0;
        for (auto &p : func_idx)
        {
          std::cout << p << " ";
          data[idx] = right[p].data();
          coding[idx] = repaired_shards[idx].data();
          idx++;
        }
        std::cout << std::endl;
        jerasure_matrix_encode(failed_data_and_parity.size(), failed_data_and_parity.size(), 8, invert.data(), data, coding, shard_size);
        for (int i = 0; i < int(failed_data_and_parity.size()); i++)
        {
          int j = 0;
          for (; j < int(new_locations_with_shard_idx.size()); j++)
          {
            if (new_locations_with_shard_idx[j].second == failed_data_and_parity[i])
            {
              break;
            }
          }
          std::string &new_node_ip = new_locations_with_shard_idx[j].first.first;
          int new_node_port = new_locations_with_shard_idx[j].first.second;
          std::string failed_shard_id = std::to_string(stripe_id * 1000 + failed_data_and_parity[i]);
          SetToMemcached(failed_shard_id.c_str(), failed_shard_id.size(), coding[i], shard_size, new_node_ip.c_str(), new_node_port);
        }
      }
      else
      {
        int help_area_size = (encode_type == Azure_LRC_1) ? (g + real_l) : g;
        int all_shard_number = (encode_type == Azure_LRC_1) ? (k + g + real_l) : (k + g);
        std::vector<char *> v_data(k);
        std::vector<char *> v_coding(help_area_size);
        char **data = (char **)v_data.data();
        char **coding = (char **)v_coding.data();
        std::vector<std::vector<char>> help_area(help_area_size, std::vector<char>(shard_size));
        std::unordered_set<int> help;
        for (auto &p : data_or_parity)
        {
          for (auto &q : p.second)
          {
            help.insert(q.first);
            if (q.first >= k)
            {
              coding[q.first - k] = q.second.data();
            }
            else
            {
              data[q.first] = q.second.data();
            }
          }
        }
        int idx = 0;
        auto erasures = std::make_shared<std::vector<int>>();
        for (int i = 0; i < k; i++)
        {
          if (help.count(i) == 0)
          {
            data[i] = help_area[idx++].data();
            erasures->push_back(i);
          }
        }
        for (int i = k; i < all_shard_number; i++)
        {
          if (help.count(i) == 0)
          {
            coding[i - k] = help_area[idx++].data();
            erasures->push_back(i);
          }
        }
        erasures->push_back(-1);

        if (encode_type == Azure_LRC_1)
        {
          if (!decode(k, g, real_l, data, coding, erasures, shard_size, encode_type, true))
          {

            std::cout << "cannot decode!" << std::endl;
          }
        }
        else
        {
          decode(k, g, 0, data, coding, erasures, shard_size, encode_type);
        }
        for (int i = 0; i < int(new_locations_with_shard_idx.size()); i++)
        {
          int failed_shard_idx = new_locations_with_shard_idx[i].second;
          if (failed_shard_idx < all_shard_number)
          {
            std::string &new_node_ip = new_locations_with_shard_idx[i].first.first;
            int new_node_port = new_locations_with_shard_idx[i].first.second;
            std::string failed_shard_id = std::to_string(stripe_id * 1000 + failed_shard_idx);
            char *transform;
            if (failed_shard_idx >= k)
            {
              transform = coding[failed_shard_idx - k];
            }
            else
            {
              transform = data[failed_shard_idx];
            }
            SetToMemcached(failed_shard_id.c_str(), failed_shard_id.size(), transform, shard_size, new_node_ip.c_str(), new_node_port);
          }
        }
      }
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
    for (int i = 0; i < helpRepairPlan->inner_az_help_shards_ip_size(); i++)
    {
      inner_az_shards_to_read.push_back({{helpRepairPlan->inner_az_help_shards_ip(i), helpRepairPlan->inner_az_help_shards_port(i)},
                                         helpRepairPlan->inner_az_help_shards_idx(i)});
    }
    int k = helpRepairPlan->k();
    int g = helpRepairPlan->g();
    int real_l = helpRepairPlan->real_l();
    int self_az_id = helpRepairPlan->self_az_id();
    bool if_partial_decoding = helpRepairPlan->if_partial_decoding();
    int stripe_id = helpRepairPlan->stripe_id();
    int shard_size = helpRepairPlan->shard_size();
    EncodeType encode_type = EncodeType(helpRepairPlan->encode_type());
    std::string main_proxy_ip = helpRepairPlan->main_proxy_ip();
    int main_proxy_port = helpRepairPlan->main_proxy_port();
    int failed_shard_idx = helpRepairPlan->failed_shard_idx();
    std::vector<int> all_failed_shards_idx;
    for (int i = 0; i < helpRepairPlan->all_failed_shards_idx_size(); i++)
    {
      all_failed_shards_idx.push_back(helpRepairPlan->all_failed_shards_idx(i));
    }
    bool merge;
    if (if_partial_decoding && one_shard_fail == false)
    {
      merge = helpRepairPlan->merge();
    }
    std::unordered_map<int, std::unordered_map<int, std::vector<char>>> data_or_parity;
    std::vector<std::thread> readers_inner_az;

    if (one_shard_fail)
    {
      for (int i = 0; i < int(inner_az_shards_to_read.size()); i++)
      {
        readers_inner_az.push_back(std::thread([&, i]()
                                               {
          std::string &ip = inner_az_shards_to_read[i].first.first;
          int port = inner_az_shards_to_read[i].first.second;
          int shard_idx = inner_az_shards_to_read[i].second;
          std::vector<char> buf(shard_size);
          std::string shard_id = std::to_string(stripe_id * 1000 + shard_idx);
          size_t temp_size;
          GetFromMemcached(shard_id.c_str(), shard_id.size(), buf.data(), &temp_size, 0, shard_size, ip.c_str(), port);
          repair_buffer_lock.lock();
          data_or_parity[self_az_id][shard_idx] = buf;
          repair_buffer_lock.unlock(); }));
      }
      for (auto &th : readers_inner_az)
      {
        th.join();
      }
      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(main_proxy_ip, std::to_string(main_proxy_port));
      asio::ip::tcp::socket socket(io_context);
      asio::connect(socket, endpoint);
      std::vector<unsigned char> int_buf_self_az_id = OppoProject::int_to_bytes(self_az_id);
      asio::write(socket, asio::buffer(int_buf_self_az_id, int_buf_self_az_id.size()));
      if (if_partial_decoding)
      {
        std::vector<char> merge_result(shard_size, 1);
        std::vector<int> matrix;
        int *coefficients = NULL;
        if (encode_type == RS && (failed_shard_idx >= k && failed_shard_idx <= (k + g - 1)))
        {
          int g_row = failed_shard_idx - k;
          std::cout << "k: " << k << " g: " << g << " failed_shard_idx: " << failed_shard_idx << std::endl;
          int *rs_matrix = reed_sol_vandermonde_coding_matrix(k, g, 8);
          matrix.resize(g * k);
          memcpy(matrix.data(), rs_matrix, g * k * sizeof(int));
          free(rs_matrix);
          for (int i = 0; i < g; i++)
          {
            for (int j = 0; j < k; j++)
            {
              std::cout << matrix[i * k + j] << " ";
            }
            std::cout << std::endl;
          }
          coefficients = matrix.data() + g_row * k;
        }
        int count = 0;
        for (auto &p : data_or_parity)
        {
          count += p.second.size();
        }
        std::vector<char *> v_data(count);
        std::vector<char *> v_coding(1);
        char **data = (char **)v_data.data();
        char **coding = (char **)v_coding.data();
        std::vector<int> new_matrix(1 * count, 1);
        int idx = 0;
        for (auto &p : data_or_parity)
        {
          for (auto &q : p.second)
          {
            data[idx] = q.second.data();
            if (encode_type == RS && (failed_shard_idx >= k && failed_shard_idx <= (k + g - 1)))
            {
              new_matrix[idx] = coefficients[q.first];
            }
            idx++;
          }
        }
        coding[0] = merge_result.data();
        jerasure_matrix_encode(count, 1, 8, new_matrix.data(), data, coding, shard_size);
        asio::write(socket, asio::buffer(merge_result, merge_result.size()));
      }
      else
      {
        std::vector<unsigned char> int_buf_num_of_shards = OppoProject::int_to_bytes(inner_az_shards_to_read.size());
        asio::write(socket, asio::buffer(int_buf_num_of_shards, int_buf_num_of_shards.size()));
        for (auto &p : data_or_parity)
        {
          for (auto &q : p.second)
          {
            std::vector<unsigned char> int_buf_shard_idx = OppoProject::int_to_bytes(q.first);
            asio::write(socket, asio::buffer(int_buf_shard_idx, int_buf_shard_idx.size()));
            asio::write(socket, asio::buffer(q.second, q.second.size()));
          }
        }
      }
      asio::error_code ignore_ec;
      socket.shutdown(asio::ip::tcp::socket::shutdown_send, ignore_ec);
      socket.close(ignore_ec);
    }
    else
    {
      int all_shard_number = (encode_type == Azure_LRC_1) ? (k + g + real_l) : (k + g);
      std::vector<int> failed_data_and_parity;

      for (int i = 0; i < int(all_failed_shards_idx.size()); i++)
      {
        if (all_failed_shards_idx[i] < all_shard_number)
        {
          failed_data_and_parity.push_back(all_failed_shards_idx[i]);
        }
      }
      std::set<int> func_idx;
      if (if_partial_decoding)
      {
        for (int i = 0; i < int(failed_data_and_parity.size()); i++)
        {
          if (failed_data_and_parity[i] >= k && failed_data_and_parity[i] < all_shard_number)
          {
            func_idx.insert(failed_data_and_parity[i]);
          }
        }
        for (int i = k; i < all_shard_number; i++)
        {
          if (func_idx.size() == failed_data_and_parity.size())
          {
            break;
          }
          func_idx.insert(i);
        }
      }
      for (int i = 0; i < int(inner_az_shards_to_read.size()); i++)
      {
        readers_inner_az.push_back(std::thread([&, i]()
                                               {
            std::string &ip = inner_az_shards_to_read[i].first.first;
            int port = inner_az_shards_to_read[i].first.second;
            int shard_idx = inner_az_shards_to_read[i].second;
            std::vector<char> buf(shard_size);
            std::string shard_id = std::to_string(stripe_id * 1000 + shard_idx);
            size_t temp_size;
            GetFromMemcached(shard_id.c_str(), shard_id.size(), buf.data(), &temp_size, 0, shard_size, ip.c_str(), port);
            repair_buffer_lock.lock();
            data_or_parity[self_az_id][shard_idx] = buf;
            repair_buffer_lock.unlock(); }));
      }
      for (auto &th : readers_inner_az)
      {
        th.join();
      }
      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(main_proxy_ip, std::to_string(main_proxy_port));
      asio::ip::tcp::socket socket(io_context);
      asio::connect(socket, endpoint);
      std::vector<unsigned char> int_buf_self_az_id = OppoProject::int_to_bytes(self_az_id);
      asio::write(socket, asio::buffer(int_buf_self_az_id, int_buf_self_az_id.size()));
      if (if_partial_decoding && merge)
      {
        std::vector<unsigned char> int_buf_num_of_shards = OppoProject::int_to_bytes(func_idx.size());
        asio::write(socket, asio::buffer(int_buf_num_of_shards, int_buf_num_of_shards.size()));

        std::vector<int> matrix;
        if (encode_type == Azure_LRC_1)
        {
          matrix.resize((g + real_l) * k);
          lrc_make_matrix(k, g, real_l, matrix.data());
        }
        else
        {
          int *rs_matrix = reed_sol_vandermonde_coding_matrix(k, g, 8);
          matrix.resize(g * k);
          memcpy(matrix.data(), rs_matrix, g * k * sizeof(int));
          free(rs_matrix);
        }
        int count = 0;
        for (auto &p : data_or_parity)
        {
          count += p.second.size();
        }
        std::vector<char> merge_result(shard_size);
        for (auto &p : func_idx)
        {
          std::vector<char *> v_data(count);
          std::vector<char *> v_coding(1);
          char **data = (char **)v_data.data();
          char **coding = (char **)v_coding.data();
          std::vector<int> new_matrix(1 * count, 1);
          int real_idx = p - k;
          int *coff = &(matrix[real_idx * k]);
          int idx = 0;
          for (auto &q : data_or_parity[self_az_id])
          {
            if (q.first < k)
            {
              new_matrix[idx] = coff[q.first];
            }
            else
            {
              new_matrix[idx] = (q.first == p);
            }
            data[idx] = q.second.data();
            idx++;
          }
          int sum = 0;
          for (auto &num : new_matrix)
          {
            sum += num;
          }
          coding[0] = merge_result.data();
          jerasure_matrix_encode(count, 1, 8, new_matrix.data(), data, coding, shard_size);
          if (sum == 0)
          {
            std::vector<char> temp(shard_size, 0);
            merge_result = temp;
          }
          std::vector<unsigned char> int_buf_func_idx = OppoProject::int_to_bytes(p);
          asio::write(socket, asio::buffer(int_buf_func_idx, int_buf_func_idx.size()));
          asio::write(socket, asio::buffer(merge_result, merge_result.size()));
        }
      }
      else
      {
        std::vector<unsigned char> int_buf_num_of_shards = OppoProject::int_to_bytes(inner_az_shards_to_read.size());
        asio::write(socket, asio::buffer(int_buf_num_of_shards, int_buf_num_of_shards.size()));
        for (auto &p : data_or_parity)
        {
          for (auto &q : p.second)
          {
            std::vector<unsigned char> int_buf_shard_idx = OppoProject::int_to_bytes(q.first);
            asio::write(socket, asio::buffer(int_buf_shard_idx, int_buf_shard_idx.size()));
            asio::write(socket, asio::buffer(q.second, q.second.size()));
          }
        }
      }
      asio::error_code ignore_ec;
      socket.shutdown(asio::ip::tcp::socket::shutdown_send, ignore_ec);
      socket.close(ignore_ec);
    }
    std::cout << "helpRepair done" << std::endl;
    return grpc::Status::OK;
  }

  grpc::Status ProxyImpl::dataProxyUpdate(
      grpc::ServerContext *context,
      const proxy_proto::DataProxyUpdatePlan *dataProxyPlan,
      proxy_proto::DataProxyReply *reply)
  {
    std::string key = dataProxyPlan->key();
    unsigned int stripeid = dataProxyPlan->stripeid();

    int update_op_id = dataProxyPlan->update_opration_id();
    std::unordered_map<int, Range> data_idx_ranges; // idx->Range
    std::vector<int> localparity_idxes;
    std::vector<std::pair<std::string, int>> data_nodes_ip_port;
    std::vector<std::pair<std::string, int>> localparity_nodes_ip_port;
    std::string collector_proxyip = dataProxyPlan->collector_proxyip();
    int collector_proxyprot = dataProxyPlan->collector_proxyport();

    for (int i = 0; i < dataProxyPlan->receive_client_shard_idx_size(); i++)
    {
      int idx = dataProxyPlan->receive_client_shard_idx(i);
      int offset = dataProxyPlan->receive_client_shard_offset(i);
      int len = dataProxyPlan->receive_client_shard_offset(i);
      data_idx_ranges[idx] = Range(offset, len);
      data_nodes_ip_port.push_back(std::make_pair(dataProxyPlan->data_nodeip(i), dataProxyPlan->data_nodeport(i)));
    }
    for (int i = 0; i < dataProxyPlan->local_parity_idx_size(); i++)
    {
      localparity_idxes.push_back(dataProxyPlan->local_parity_idx(i));
      localparity_nodes_ip_port.push_back(std::make_pair(dataProxyPlan->local_parity_nodeip(i), dataProxyPlan->local_parity_nodeport(i)));
    }
    // may need to call repair

    auto receive_update = [this, key, stripeid, update_op_id, data_idx_ranges, data_nodes_ip_port, localparity_idxes, localparity_nodes_ip_port, collector_proxyip, collector_proxyprot]() mutable {

    };

    return grpc::Status::OK;
  }

  grpc::Status ProxyImpl::collectorProxyUpdate(
      grpc::ServerContext *context,
      const proxy_proto::CollectorProxyUpdatePlan *collectorProxyPlan,
      proxy_proto::CollectorProxyReply *reply)
  {
    return grpc::Status::OK;
  }

} // namespace OppoProject
