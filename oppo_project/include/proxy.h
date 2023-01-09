#ifndef PROXY_H
#define PROXY_H
#include "coordinator.grpc.pb.h"
#include "devcommon.h"
#include "proxy.grpc.pb.h"
#include <asio.hpp>
#include <grpc++/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <libmemcached/memcached.h>
#include <thread>
namespace OppoProject {
class ProxyImpl final
    : public proxy_proto::proxyService::Service,
      public std::enable_shared_from_this<OppoProject::ProxyImpl> {

public:
  ProxyImpl() {
    init_coordinator();
    init_memcached();
  }
  ~ProxyImpl() { memcached_free(m_memcached); };
  grpc::Status checkalive(grpc::ServerContext *context,
                          const proxy_proto::CheckaliveCMD *request,
                          proxy_proto::RequestResult *response) override;
  grpc::Status EncodeAndSetObject(
      grpc::ServerContext *context,
      const proxy_proto::ObjectAndPlacement *object_and_placement,
      proxy_proto::SetReply *response) override;
  grpc::Status decodeAndGetObject(
      grpc::ServerContext *context,
      const proxy_proto::ObjectAndPlacement *object_and_placement,
      proxy_proto::GetReply *response) override;

private:
  bool init_memcached();
  bool init_coordinator();
  std::unique_ptr<coordinator_proto::CoordinatorService::Stub>
      m_coordinator_stub;
  bool SetToMemcached(const char *key, size_t key_length, const char *value,
                      size_t value_length);
  bool GetFromMemcached(const char *key, size_t key_length, char *value,
                        size_t *value_length);
  memcached_st *m_memcached;
};

class Proxy {
public:
  Proxy() {}
  void Run() {
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    grpc::ServerBuilder builder;
    std::string proxy_ip_port = "localhost:50055";
    std::cout << "proxy_ip_port:" << proxy_ip_port << std::endl;
    builder.AddListeningPort(proxy_ip_port, grpc::InsecureServerCredentials());
    builder.RegisterService(&m_proxyImpl_ptr);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    server->Wait();
  }

private:
  // std::shared_ptr<OppoProject::ProxyImpl> m_proxyImpl_ptr;
  OppoProject::ProxyImpl m_proxyImpl_ptr;
};
} // namespace OppoProject
#endif