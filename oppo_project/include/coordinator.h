#ifndef COORDINATOR_H
#define COORDINATOR_H
#include "coordinator.grpc.pb.h"
#include "proxy.grpc.pb.h"
#include <grpc++/create_channel.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <meta_definition.h>
#include <mutex>

namespace OppoProject {
class CoordinatorImpl final
    : public coordinator_proto::CoordinatorService::Service {
public:
  CoordinatorImpl() {
    std::string proxy_ip_port = "localhost:50055";
    auto _stub = proxy_proto::proxyService::NewStub(
        grpc::CreateChannel(proxy_ip_port, grpc::InsecureChannelCredentials()));
    proxy_proto::CheckaliveCMD Cmd;
    proxy_proto::RequestResult result;
    grpc::ClientContext clientContext;
    Cmd.set_name("wwwwwwwww");
    grpc::Status status;
    status = _stub->checkalive(&clientContext, Cmd, &result);
    if (status.ok()) {
      std::cout << "checkalive,ok" << std::endl;
    }
    m_proxy_ptrs.insert(std::make_pair(proxy_ip_port, std::move(_stub)));
  }
  grpc::Status sayHelloToCoordinator(
      ::grpc::ServerContext *context,
      const coordinator_proto::RequestToCoordinator *helloRequestToCoordinator,
      coordinator_proto::ReplyFromCoordinator *helloReplyFromCoordinator)
      override;
  grpc::Status uploadOriginKeyValue(
      grpc::ServerContext *context,
      const coordinator_proto::RequestProxyIPPort *keyValueSize,
      coordinator_proto::ReplyProxyIPPort *proxyIPPort) override;

  grpc::Status checkalive(
      grpc::ServerContext *context,
      const coordinator_proto::RequestToCoordinator *helloRequestToCoordinator,
      coordinator_proto::ReplyFromCoordinator *helloReplyFromCoordinator)
      override;
  grpc::Status
  reportCommitAbort(grpc::ServerContext *context,
                    const coordinator_proto::CommitAbortKey *commit_abortkey,
                    coordinator_proto::ReplyFromCoordinator
                        *helloReplyFromCoordinator) override;
  grpc::Status
  checkCommitAbort(grpc::ServerContext *context,
                   const coordinator_proto::AskIfSetSucess *key,
                   coordinator_proto::RepIfSetSucess *reply) override;
  grpc::Status
  getValue(::grpc::ServerContext *context,
           const coordinator_proto::KeyAndClientIP *keyValueSize,
           coordinator_proto::RepIfGetSucess *proxyIPPort) override;

private:
  std::mutex m_mutex;
  int m_next_stripe_id = 0;
  std::map<std::string, std::unique_ptr<proxy_proto::proxyService::Stub>>
      m_proxy_ptrs;
  std::unordered_map<std::string, ObjectItemBigSmall>
      m_object_table_big_small_updating;
  std::unordered_map<std::string, ObjectItemBigSmall>
      m_object_table_big_small_commit;
  ECSchema m_encode_parameter;
};

class Coordinator {
public:
  // Coordinator
  void Run() {
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    grpc::ServerBuilder builder;
    std::string server_address("0.0.0.0:50051");
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&m_coordinatorImpl);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
  }

private:
  OppoProject::CoordinatorImpl m_coordinatorImpl;
};
} // namespace OppoProject

#endif