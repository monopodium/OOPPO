#include "client.h"
#include "coordinator.grpc.pb.h"

#include <asio.hpp>
namespace OppoProject
{
  std::string Client::sayHelloToCoordinatorByGrpc(std::string hello)
  {
    coordinator_proto::RequestToCoordinator request;
    request.set_name(hello);
    coordinator_proto::ReplyFromCoordinator reply;
    grpc::ClientContext context;
    grpc::Status status =
        m_coordinator_ptr->sayHelloToCoordinator(&context, request, &reply);
    if (status.ok())
    {
      return reply.message();
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }
  bool Client::SetParameterByGrpc(ECSchema input_ecschema)
  {
    /*待补充，通过这个函数，要能设置coordinator的编码参数*/
    /*编码参数存储在变量 m_encode_parameter中*/
    coordinator_proto::Parameter parameter;
    parameter.set_partial_decoding((int)input_ecschema.partial_decoding);
    parameter.set_encodetype((int)input_ecschema.encodetype);
    parameter.set_placementtype(input_ecschema.placementtype);
    parameter.set_k_datablock(input_ecschema.k_datablock);
    parameter.set_real_l_localgroup(input_ecschema.real_l_localgroup);
    parameter.set_g_m_globalparityblock(input_ecschema.g_m_globalparityblock);
    parameter.set_b_datapergoup(input_ecschema.b_datapergoup);
    parameter.set_small_file_upper(input_ecschema.small_file_upper);
    parameter.set_blob_size_upper(input_ecschema.blob_size_upper);
    grpc::ClientContext context;
    coordinator_proto::RepIfSetParaSucess reply;
    grpc::Status status = m_coordinator_ptr->setParameter(&context, parameter, &reply);
    if (status.ok())
    {
      return reply.ifsetparameter();
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return false;
    }
  }
  bool Client::set(std::string key, std::string value, std::string flag)
  {

    /* grpc*/
    grpc::ClientContext get_proxy_ip_port;
    coordinator_proto::RequestProxyIPPort request;
    coordinator_proto::ReplyProxyIPPort reply;
    request.set_key(key);
    request.set_valuesizebytes(value.size());
    grpc::Status status = m_coordinator_ptr->uploadOriginKeyValue(
        &get_proxy_ip_port, request, &reply);

    /* grpc*/
    if (!status.ok())
    {
      std::cout << "upload stripe failed!" << std::endl;
      return false;
    }
    else
    {

      std::string proxy_ip = reply.proxyip();
      int proxy_port = reply.proxyport();
      std::cout << "proxy_ip:" << proxy_ip << std::endl;
      std::cout << "proxy_port:" << proxy_port << std::endl;
      asio::io_context io_context;

      asio::error_code error;
      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::resolver::results_type endpoints =
          resolver.resolve(proxy_ip, std::to_string(proxy_port));

      asio::ip::tcp::socket sock_data(io_context);
      asio::connect(sock_data, endpoints);

      std::cout << "key.size()" << key.size() << std::endl;
      std::cout << "value.size()" << value.size() << std::endl;
      std::cout << "proxy_ip:"<<proxy_ip<<std::endl;
      std::cout << "proxy_port:"<<proxy_port<<std::endl;
      asio::write(sock_data, asio::buffer(key, key.size()), error);
      std::cout<<"no error"<<std::endl;
      asio::write(sock_data, asio::buffer(value, value.size()), error);
      std::cout<<"no error!!!!!"<<std::endl;
      asio::error_code ignore_ec;
      sock_data.shutdown(asio::ip::tcp::socket::shutdown_send, ignore_ec);
      sock_data.close(ignore_ec);

      /*这里需要通过检查元数据object_table_big_small_commit来确认是否存成功*/
      grpc::ClientContext check_commit;
      coordinator_proto::AskIfSetSucess request;
      request.set_key(key);
      coordinator_proto::RepIfSetSucess reply;
      grpc::Status status;
      status =
          m_coordinator_ptr->checkCommitAbort(&check_commit, request, &reply);
      std::cout<<"m_coordinator_ptr error!!!!!"<<std::endl;
      if (status.ok())
      {
        if (reply.ifcommit())
        {
          return true;
        }
        else
        {
          std::cout << key << " not commit!!!!!";
        }
      }
      else
      {
        std::cout << key << " Fail to check!!!!!";
      }
    }
    return false;
    /* grpc*/
  }
  bool Client::get(std::string key, std::string &value)
  {
    grpc::ClientContext context;
    coordinator_proto::KeyAndClientIP request;
    request.set_key(key);
    request.set_clientip(m_clientIPForGet);
    request.set_clientport(m_clientPortForGet);
    coordinator_proto::RepIfGetSucess reply;
    grpc::Status status;

    status = m_coordinator_ptr->getValue(&context, request, &reply);

    std::cout<<"get 1"<<std::endl;
    asio::ip::tcp::socket socket_data(io_context);
    int value_size = reply.valuesizebytes();
    
    acceptor.accept(socket_data);
    asio::error_code error;
    std::vector<char> buf_key(key.size());
    std::vector<char> buf(value_size);
    
    size_t len = asio::read(socket_data, asio::buffer(buf_key, key.size()), error);
    std::cout<<"get 2"<<std::endl;
    int flag = 1;
    for (int i = 0; i < int(key.size()); i++)
    {
      if (key[i] != buf_key[i])
      {
        flag = 0;
      }
    }
    std::cout << "value_size:" << value_size << std::endl;
    std::cout << "flag:" << flag << std::endl;
    if (flag)
    {
      len = asio::read(socket_data, asio::buffer(buf, value_size), error);
    }
    asio::error_code ignore_ec;
    socket_data.shutdown(asio::ip::tcp::socket::shutdown_receive, ignore_ec);
    socket_data.close(ignore_ec);
    std::cout << "get key: " << key << " valuesize: " << len << std::endl;
    // for (const auto &c : buf) {
    //   std::cout << c;
    // }
    value = std::string(buf.data(), buf.size());
    std::cout << std::endl;
    return true;
  }
  bool Client::repair(std::vector<int> failed_node_list)
  {
    grpc::ClientContext context;
    coordinator_proto::FailNodes request;
    coordinator_proto::RepIfRepairSucess reply;
    for (int &node : failed_node_list)
    {
      request.add_node_list(node);
    }
    grpc::Status status = m_coordinator_ptr->requestRepair(&context, request, &reply);
    return true;
  }

  bool Client::update(std::string key, int offset, int length)
  {
    grpc::ClientContext grpccontext;
    coordinator_proto::UpdatePrepareRequest request;
    coordinator_proto::UpdateDataLocation data_location;

    grpc::Status status = m_coordinator_ptr->updateGetLocation(&grpccontext, request, &data_location);
    if (status.ok())
    {
      if (key != data_location.key())
        std::cerr << "what!!! coordinator returns other object's update soluuution!!" << std::endl;

      int descending_len = length;
      std::vector<std::pair<std::string, int>> proxy_ipports;
      std::vector<int> each_proxy_num;
      int i = 0;
      int j = 0;
      for (i = 0; i < data_location.proxyip_size(); i++)
      {
        std::string proxy_ip = data_location.proxyip(i);
        int proxy_port = data_location.proxyport(i);
        int count = data_location.num_each_proxy(i);
        asio::io_context io_context;
        asio::error_code error;
        asio::ip::tcp::resolver resolver(io_context);
        asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(proxy_ip, std::to_string(proxy_port));
        asio::ip::tcp::socket data_socket(io_context);
        asio::connect(data_socket, endpoint);

        for (int t = 0; t < count; t++)
        {
          int idx = data_location.datashardidx(j);
          int offset_in_shard = data_location.offsetinshard(j);
          int length_in_shard = data_location.lengthinshard(j);
          j++; //!!
          // asio::write(data_socket,asio::buffer(idx,sizeof(int)),error);
          // asio::write(data_socket,asio::buffer(offset_in_shard,sizeof(int)),error);
          // asio::write(data_socket,asio::buffer(length_in_shard,sizeof(int)),error);
        }
      }
    }

    return true;
  }
} // namespace OppoProject