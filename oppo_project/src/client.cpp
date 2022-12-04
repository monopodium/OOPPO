#include "client.h"
#include "coordinator.grpc.pb.h"


namespace OppoProject{
    std::string Client::sayHelloToCoordinatorByGrpc(std::string hello){
        coordinator_proto::RequestToCoordinator request;
        request.set_name(hello);
        coordinator_proto::ReplyFromCoordinator reply;
        grpc::ClientContext context;
        grpc::Status status = m_coordinator_ptr->sayHelloToCoordinator(&context, request, &reply);
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

}

// int main(int argc, char *argv[])
// {
//     try
//     {
//         if (argc != 2)
//         {
//             std::cerr << "Usage: client <host>" << std::endl;
//             return 1;
//         }

//         asio::io_context io_context;

//         tcp::resolver resolver(io_context);
//         tcp::resolver::results_type endpoints =
//             resolver.resolve(argv[1], "11311");

//         tcp::socket socket(io_context);
//         asio::connect(socket, endpoints);

//         for (;;)
//         {
//             std::vector<char> buf(128);
//             asio::error_code error;

//             size_t len = socket.read_some(asio::buffer(buf), error);

//             if (error == asio::error::eof)
//                 break; // Connection closed cleanly by peer.
//             else if (error)
//                 throw asio::system_error(error); // Some other error.

//             std::cout.write(buf.data(), len);
//         }
//     }
//     catch (std::exception &e)
//     {
//         std::cerr << e.what() << std::endl;
//     }
//     return 0;
// }
