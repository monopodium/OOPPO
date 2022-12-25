#include "client.h"
#include "coordinator.grpc.pb.h"
#include "devcommon.h"

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
  }

  /* grpc*/
}

} // namespace OppoProject