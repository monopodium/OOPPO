#ifndef CLIENT_H
#define CLIENT_H

#ifdef BAZEL_BUILD
#include "src/proto/coordinator.grpc.pb.h"
#else
#include "coordinator.grpc.pb.h"
#endif

#include "meta_definition.h"
#include <grpcpp/grpcpp.h>
namespace OppoProject {
class Client {
  std::string m_coordinatorIpPort = "localhost:50051";

public:
  Client() {
    auto channel = grpc::CreateChannel(m_coordinatorIpPort,
                                       grpc::InsecureChannelCredentials());
    m_coordinator_ptr = coordinator_proto::CoordinatorService::NewStub(channel);
  }
  std::string sayHelloToCoordinatorByGrpc(std::string hello);
  bool set(std::string key, std::string value, std::string flag);

private:
  std::unique_ptr<coordinator_proto::CoordinatorService::Stub>
      m_coordinator_ptr;
};

} // namespace OppoProject

#endif // CLIENT_H