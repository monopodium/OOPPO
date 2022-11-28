#include <iostream>
#include <memory>
#include <string>
#include <asio.hpp>
#include <iostream>
#include <string.h>
#include <functional>
#include <vector>
#include <memory>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#ifdef BAZEL_BUILD
#include "src/proto/coordinator.grpc.pb.h"
#else
#include "coordinator.grpc.pb.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
// #include <libmemcached/memcached.h>//C 接口
// #include <libmemcached/util.h>  //工具，包括pool功能
// #include <libmemcached/memcached.hpp> //将c接口封装成的c++接口，
#include <libmemcached/memcached.h>   //C 接口
#include <libmemcached/util.h>        //工具，包括pool功能
#include <libmemcached/memcached.hpp> //将c接口封装成的c++接口，
using namespace std;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

using asio::ip::tcp;

std::string make_daytime_string()
{
    using namespace std; // For time_t, time and ctime;
    time_t now = time(0);
    return ctime(&now);
}

// Logic and data behind the server's behavior.
class GreeterServiceImpl final : public Greeter::Service
{
    Status SayHello(ServerContext *context, const HelloRequest *request,
                    HelloReply *reply) override
    {
        std::string prefix("Hello ");
        reply->set_message(prefix + request->name());
        return Status::OK;
    }
};

void RunServer()
{
    std::string server_address("0.0.0.0:50051");
    GreeterServiceImpl service;

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);
    // Finally assemble the server.
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();
}

void test_libmemcached()
{
    std::string value;
    try
    {
        asio::io_context io_context;

        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 11311));

        for (;;)
        {
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            std::string message = make_daytime_string();
            value = message;

            asio::error_code ignored_error;
            asio::write(socket, asio::buffer(message), ignored_error);
            break;
        }
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    memcached_st *memc;
    memcached_return rc;
    memcached_server_st *servers;

    memc = memcached_create(NULL);

    servers = memcached_server_list_append_with_weight(NULL, (char *)"localhost", 11211, 50, &rc);
    servers = memcached_server_list_append_with_weight(servers, (char *)"localhost", 11212, 50, &rc);

    rc = memcached_server_push(memc, servers);

    memcached_server_free(servers);

    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA);

    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_REMOVE_FAILED_SERVERS, true); // set失败，则快隔离将其标记为dead
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_DEAD_TIMEOUT, 5);             //慢恢复，每隔多少s试探一次即可。

    int time_sl = 0;
    int times = 0;

    while (times++ < 10)
    {
        size_t len = value.size();
        rc = memcached_set(memc, "zhaoritian", 10, value.c_str(), len, (time_t)180, (uint32_t)0);
        char *result = memcached_get(memc, "zhaoritian", 10, &len, NULL, &rc);
        std::string ret(result);
        cout << "zhaoritian: " << ret << endl;
        // save data
        const char *keys[] = {"key1", "key2", "key3", "key4", "key5", "key6", "key7"};
        const size_t key_length[] = {4, 4, 4, 4, 4, 4, 4};
        const char *values[] = {"key1Value", "key2Value", "key3Value", "key4Value", "key5Value", "key6Value", "key7Value"};
        size_t val_length[] = {sizeof(values[0]), sizeof(values[1]), sizeof(values[2]), sizeof(values[3]), sizeof(values[4]), sizeof(values[5]), sizeof(values[6])};
        for (int i = 0; i < 7; i++)
        {
            rc = memcached_set(memc, keys[i], key_length[i], values[i], val_length[i], (time_t)180, (uint32_t)0);
            printf("key: %s  rc:%s\n", keys[i], memcached_strerror(memc, rc)); // 输出状态

            char *result = memcached_get(memc, keys[i], key_length[i], NULL, NULL, &rc);
            if (rc == MEMCACHED_SUCCESS)
            {
                cout << "Get value:" << result << " sucessful!" << endl;
            }
            else
                cout << memcached_strerror(memc, rc) << endl;
        }
        printf("time: %d\n", time_sl++);
        sleep(1);
    }
    memcached_free(memc);
}
int main(int argc, char **argv)
{
    test_libmemcached();
    RunServer();

    return 0;
}