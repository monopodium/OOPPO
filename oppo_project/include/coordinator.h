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
  CoordinatorImpl(

  ) {}
  grpc::Status setParameter(
      ::grpc::ServerContext *context,
      const coordinator_proto::Parameter *parameter,
      coordinator_proto::RepIfSetParaSucess *setParameterReply)
      override;
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
           const coordinator_proto::KeyAndClientIP *keyClient,
           coordinator_proto::RepIfGetSucess *getReplyClient) override;
  bool init_AZinformation(std::string Azinformation_path);
  bool init_proxy(std::string proxy_information_path);
  void
  generate_placement(std::vector<std::pair<std::string, int>> datanodeip_port);

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
  std::map<int, AZitem> m_AZ_info;
  std::map<int, Nodeitem> m_Node_info;
};

class Coordinator {
public:
  Coordinator(
      std::string m_coordinator_ip_port = "0.0.0.0:50051",
      std::string m_Azinformation_path =
          "/home/ms/temp_test/OOPPO/oppo_project/config/AZInformation.xml")
      : m_coordinator_ip_port{m_coordinator_ip_port},
        m_Azinformation_path{m_Azinformation_path} {
    m_coordinatorImpl.init_AZinformation(m_Azinformation_path);
    m_coordinatorImpl.init_proxy(m_Azinformation_path);
  };
  // Coordinator
  void Run() {
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    grpc::ServerBuilder builder;
    std::string server_address(m_coordinator_ip_port);
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&m_coordinatorImpl);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
  }

private:
  std::string m_Azinformation_path =
      "/home/ms/temp_test/OOPPO/oppo_project/config/AZInformation.xml";
  std::string m_coordinator_ip_port = "0.0.0.0:50051";
  OppoProject::CoordinatorImpl m_coordinatorImpl;
};
} // namespace OppoProject

#endif