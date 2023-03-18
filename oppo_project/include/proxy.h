#ifndef PROXY_H
#define PROXY_H
#include "coordinator.grpc.pb.h"
#include "devcommon.h"
#include "proxy.grpc.pb.h"
#include "meta_definition.h"
#include "azure_lrc.h"
#include <asio.hpp>
#include <grpc++/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <libmemcached/memcached.h>
#include <thread>
#include <semaphore.h>
namespace OppoProject
{
  class ProxyImpl final
      : public proxy_proto::proxyService::Service,
        public std::enable_shared_from_this<OppoProject::ProxyImpl>
  {

  public:
    ProxyImpl(std::string proxy_ip_port, std::string config_path, std::string coordinator_ip) : config_path(config_path), proxy_ip_port(proxy_ip_port), acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::address::from_string(proxy_ip_port.substr(0, proxy_ip_port.find(':')).c_str()), 1 + std::stoi(proxy_ip_port.substr(proxy_ip_port.find(':') + 1, proxy_ip_port.size())))), coordinator_ip(coordinator_ip)
    {
      init_coordinator();
    }
    ~ProxyImpl() { memcached_free(m_memcached); };
    grpc::Status checkalive(grpc::ServerContext *context,
                            const proxy_proto::CheckaliveCMD *request,
                            proxy_proto::RequestResult *response) override;
    grpc::Status EncodeAndSetObject(
        grpc::ServerContext *context,
        const proxy_proto::ObjectAndPlacement *object_and_placement,
        proxy_proto::SetReply *response) override;
    grpc::Status WriteBufferAndEncode(
        grpc::ServerContext *context,
        const proxy_proto::ObjectAndPlacement *object_and_placement,
        proxy_proto::SetReply *response) override;
    grpc::Status decodeAndGetObject(
        grpc::ServerContext *context,
        const proxy_proto::ObjectAndPlacement *object_and_placement,
        proxy_proto::GetReply *response) override;
    grpc::Status getObjectFromBuffer(
        grpc::ServerContext *context,
        const proxy_proto::ObjectAndPlacement *object_and_placement,
        proxy_proto::GetReply *response) override;
    grpc::Status mainRepair(
        grpc::ServerContext *context,
        const proxy_proto::mainRepairPlan *mainRepairPlan,
        proxy_proto::mainRepairReply *reply) override;
    grpc::Status helpRepair(
        grpc::ServerContext *context,
        const proxy_proto::helpRepairPlan *helpRepairPlan,
        proxy_proto::helpRepairReply *reply) override;

    grpc::Status dataProxyUpdate(
        grpc::ServerContext *context,
        const proxy_proto::DataProxyUpdatePlan *dataProxyPlan,
        proxy_proto::DataProxyReply *reply) override;
    grpc::Status collectorProxyUpdate(
        grpc::ServerContext *context,
        const proxy_proto::CollectorProxyUpdatePlan *collectorProxyPlan,
        proxy_proto::CollectorProxyReply *reply) override;

  private:
    bool init_coordinator();
    std::unique_ptr<coordinator_proto::CoordinatorService::Stub>
        m_coordinator_stub;
    bool SetToMemcached(const char *key, size_t key_length, const char *value, size_t value_length, const char *ip, int port);
    bool SetToMemcached(const char *key, size_t key_length, size_t offset, const char *value, size_t value_length, const char *ip, int port);
    bool GetFromMemcached(const char *key, size_t key_length, char *value, size_t *value_length, int offset, int lenth, const char *ip, int port);
    std::string config_path;
    memcached_st *m_memcached;
    std::string proxy_ip_port;
    std::mutex memcached_lock;
    asio::io_context io_context;
    asio::ip::tcp::acceptor acceptor;
    std::mutex repair_buffer_lock;
    std::mutex proxybuf_lock;
    std::vector<std::vector<char>> proxy_buf;
    std::vector<int> buf_offset;
    sem_t sem;
    std::string coordinator_ip;
  };

  class Proxy
  {
  public:
    Proxy(std::string proxy_ip_port, std::string config_path, std::string coordinator_ip) : proxy_ip_port(proxy_ip_port), m_proxyImpl_ptr(proxy_ip_port, config_path, coordinator_ip) {}
    void Run()
    {
      grpc::EnableDefaultHealthCheckService(true);
      grpc::reflection::InitProtoReflectionServerBuilderPlugin();
      grpc::ServerBuilder builder;
      std::cout << "proxy_ip_port:" << proxy_ip_port << std::endl;
      builder.AddListeningPort(proxy_ip_port, grpc::InsecureServerCredentials());
      builder.RegisterService(&m_proxyImpl_ptr);
      std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
      server->Wait();
    }

  private:
    // std::shared_ptr<OppoProject::ProxyImpl> m_proxyImpl_ptr;
    std::string proxy_ip_port;
    OppoProject::ProxyImpl m_proxyImpl_ptr;
  };
} // namespace OppoProject
#endif