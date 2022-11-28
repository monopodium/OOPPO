#include "coordinator.h"



namespace OppoProject{
    grpc::Status CoordinatorImpl::sayHelloToCoordinator(::grpc::ServerContext *context,
                    const coordinator_proto::RequestToCoordinator *helloRequestToCoordinator,
                    coordinator_proto::ReplyFromCoordinator *helloReplyFromCoordinator)
    {
        std::string prefix("Hello ");
        helloReplyFromCoordinator->set_message(prefix + helloRequestToCoordinator->name());
        std::cout << prefix + helloRequestToCoordinator->name() << std::endl;
        return grpc::Status::OK;
    }


}