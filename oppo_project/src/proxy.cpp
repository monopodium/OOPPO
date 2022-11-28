#include "proxy.h"
namespace OppoProject{
grpc::Status ProxyImpl::checkalive(grpc::ServerContext *context, const proxy_proto::CheckaliveCMD *request,
                                    proxy_proto::RequestResult *response){
                
        std::cout << "checkalive"<<request->name() << std::endl;
        response->set_message(false);
        return grpc::Status::OK;
    }

}