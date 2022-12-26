#include "client.h"
#include "coordinator.grpc.pb.h"

#include <asio.hpp>
namespace OppoProject {
std::string Client::sayHelloToCoordinatorByGrpc(std::string hello) {
  coordinator_proto::RequestToCoordinator request;
  request.set_name(hello);
  coordinator_proto::ReplyFromCoordinator reply;
  grpc::ClientContext context;
  grpc::Status status =
      m_coordinator_ptr->sayHelloToCoordinator(&context, request, &reply);
  if (status.ok()) {
    return reply.message();
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
    return "RPC failed";
  }
}
bool Client::SetParameter(ECSchema input_ecschema) {
  /*待补充，通过这个函数，要能设置coordinator的编码参数*/
  /*编码参数存储在变量 m_encode_parameter中*/
  return true;
}
bool Client::set(std::string key, std::string value, std::string flag) {

  /* grpc*/
  grpc::ClientContext get_proxy_ip_port;
  coordinator_proto::RequestProxyIPPort request;
  coordinator_proto::ReplyProxyIPPort reply;
  request.set_key(key);
  request.set_valuesizebytes(value.size());
  grpc::Status status = m_coordinator_ptr->uploadOriginKeyValue(
      &get_proxy_ip_port, request, &reply);

  /* grpc*/
  if (!status.ok()) {
    std::cout << "upload stripe failed!" << std::endl;
    return false;
  } else {

    std::string proxy_ip = reply.proxyip();
    short proxy_port = reply.proxyport();
    std::cout << "proxy_ip:" << proxy_ip << std::endl;
    std::cout << "proxy_port:" << proxy_port << std::endl;
    asio::io_context io_context;

    asio::error_code error;
    asio::ip::tcp::resolver resolver(io_context);
    asio::ip::tcp::resolver::results_type endpoints =
        resolver.resolve(proxy_ip, "12233");

    asio::ip::tcp::socket sock_data(io_context);
    asio::connect(sock_data, endpoints);

    std::cout << "key.size()" << key.size() << std::endl;
    std::cout << "value.size()" << value.size() << std::endl;
    asio::write(sock_data, asio::buffer(key, key.size()), error);
    asio::write(sock_data, asio::buffer(value, value.size()), error);
    if (error == asio::error::eof) {
      std::cout << "error ==asio::error::eof "
                << std::endl; // Connection closed cleanly by peer.
    } else if (error) {
      throw asio::system_error(error); // Some other error.
    }

    /*这里需要通过检查元数据object_table_big_small_commit来确认是否存成功*/
    grpc::ClientContext check_commit;
    coordinator_proto::AskIfSetSucess request;
    request.set_key(key);
    coordinator_proto::RepIfSetSucess reply;
    grpc::Status status;
    status =
        m_coordinator_ptr->checkCommitAbort(&check_commit, request, &reply);
    if (status.ok()) {
      if (reply.ifcommit()) {
        return true;
      } else {
        std::cout << key << " not commit!!!!!";
      }
    } else {
      std::cout << key << "Fail to check!!!!!";
    }
  }
  return false;
  /* grpc*/
}

} // namespace OppoProject