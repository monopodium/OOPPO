#include "proxy.h"
#include "jerasure.h"
#include "reed_sol.h"
#include <libmemcached/memcached.h>
#include <thread>

#include <string>
template <typename T> inline T ceil(T const &A, T const &B) {
  return T((A + B - 1) / B);
};
namespace OppoProject {
grpc::Status ProxyImpl::checkalive(grpc::ServerContext *context,
                                   const proxy_proto::CheckaliveCMD *request,
                                   proxy_proto::RequestResult *response) {

  std::cout << "checkalive" << request->name() << std::endl;
  response->set_message(false);
  init_coordinator();
  return grpc::Status::OK;
}
bool setToMemcached(const char *key, size_t key_length, const char *value,
                    size_t value_length, bool read) {

  memcached_st *memc;
  memcached_return rc;
  memcached_server_st *servers;
  memc = memcached_create(NULL);

  servers = memcached_server_list_append(NULL, (char *)"localhost", 8100, &rc);
  servers =
      memcached_server_list_append(servers, (char *)"localhost", 8101, &rc);
  servers =
      memcached_server_list_append(servers, (char *)"localhost", 8102, &rc);
  servers =
      memcached_server_list_append(servers, (char *)"localhost", 8103, &rc);
  servers =
      memcached_server_list_append(servers, (char *)"localhost", 8104, &rc);
  servers =
      memcached_server_list_append(servers, (char *)"localhost", 8105, &rc);
  rc = memcached_server_push(memc, servers);
  memcached_server_free(servers);

  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_DISTRIBUTION,
                         MEMCACHED_DISTRIBUTION_CONSISTENT);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_RETRY_TIMEOUT, 20);
  //  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_REMOVE_FAILED_SERVERS, 1)
  //  ;  // 同时设置MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT 和
  //  MEMCACHED_BEHAVIOR_AUTO_EJECT_HOSTS
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT, 5);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_AUTO_EJECT_HOSTS, true);

  rc = memcached_set(memc, key, key_length, value, value_length, (time_t)0,
                     (uint32_t)0);
  memcached_free(memc);
  return true;
}
bool encode(int k, int m, char **data_ptrs, char **coding_ptrs, int blocksize) {
  int *matrix;
  matrix = reed_sol_vandermonde_coding_matrix(k, m, 8);
  jerasure_matrix_encode(k, m, 8, matrix, data_ptrs, coding_ptrs, blocksize);
  return true;
}
grpc::Status ProxyImpl::EncodeAndSetObject(
    grpc::ServerContext *context,
    const proxy_proto::ObjectAndPlacement *object_and_placement,
    proxy_proto::SetReply *response) {

  std::string key = object_and_placement->key();
  int value_size_bytes = object_and_placement->valuesizebyte();
  int k = object_and_placement->k();
  int m = object_and_placement->m();
  int blocksizebyte = object_and_placement->blocksizebyte();

  for (auto shardid = object_and_placement->shardid().cbegin();
       shardid != object_and_placement->shardid().cend(); shardid++) {
    std::cout << *shardid << std::endl;
  }

  auto encode_and_save = [this, key, value_size_bytes, k, m, blocksizebyte,
                          object_and_placement]() mutable {
    asio::io_context io_context;
    asio::ip::tcp::acceptor acceptor(
        io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 12233));
    asio::ip::tcp::socket socket_data(io_context);
    acceptor.accept(socket_data);
    asio::error_code error;

    std::cout << "key.size() " << key.size() << std::endl;

    int extend_value_size_byte =
        blocksizebyte * ceil(value_size_bytes, blocksizebyte);
    std::cout << "extend_value_size_byte:" << extend_value_size_byte
              << std::endl;
    std::vector<char> buf_key(key.size());
    std::vector<char> buf(extend_value_size_byte);

    for (int i = value_size_bytes; i < extend_value_size_byte; i++) {
      buf[i] = '0';
    }
    size_t len =
        socket_data.read_some(asio::buffer(buf_key, key.size()), error);

    if (error == asio::error::eof) {
      std::cout << "error == asio::error::eof" << std::endl;
    } else if (error) {
      throw asio::system_error(error);
    }
    for (int i = 0; i < key.size(); i++) {
      std::cout << buf_key[i];
    }
    std::cout << std::endl;
    int flag = 1;
    for (int i = 0; i < key.size(); i++) {
      if (key[i] != buf_key[i]) {
        flag = 0;
      }
    }
    if (flag) {
      len = socket_data.read_some(asio::buffer(buf, value_size_bytes), error);
      std::cout << len << std::endl;
    }
    for (const auto &c : buf) {
      std::cout << c;
    }
    std::cout << std::endl;
    // this->

    /*将块切分！*/
    char **data = (char **)malloc(sizeof(char *) * k);
    char **coding = (char **)malloc(sizeof(char *) * m);

    for (int i = 0; i * blocksizebyte < extend_value_size_byte; i = i + 1) {
      data[i] = &buf[i * blocksizebyte];
    }

    for (int i = 0; i < m; i++) {
      coding[i] = (char *)malloc(sizeof(char) * blocksizebyte);
      if (coding[i] == NULL) {
        perror("malloc");
        exit(1);
      }
    }

    encode(k, m, data, coding, blocksizebyte);

    for (int i = 0; i < blocksizebyte; i++) {
      std::cout << coding[0][i];
    }
    std::cout << std::endl;

    for (int i = 0; i < k + m; i++) {
      std::string shardid_str =
          std::to_string(object_and_placement->shardid()[i]);
      // char *shardid_str = (char *)malloc(sizeof(char) * 64);
      // s(object_and_placement->shardid()[i], shardid_str, 10);
      unsigned int a;
      std::cout << (object_and_placement->shardid()[i]) << std::endl;
      std::cout << shardid_str << std::endl;
      if (i < k) {
        setToMemcached(shardid_str.c_str(), 64, data[i], blocksizebyte, false);
      } else {
        setToMemcached(shardid_str.c_str(), 64, coding[i - k], blocksizebyte,
                       false);
      }
    }

    coordinator_proto::CommitAbortKey commit_abort_key;
    coordinator_proto::ReplyFromCoordinator result;
    grpc::ClientContext context;
    commit_abort_key.set_key(key);
    commit_abort_key.set_ifcommitmetadata(true);
    grpc::Status status;
    status = this->m_coordinator_stub->reportCommitAbort(
        &context, commit_abort_key, &result);
    if (status.ok()) {
      std::cout << "connect coordinator,ok" << std::endl;

    } else {
      std::cout << "oooooH coordinator,fail!!!!" << std::endl;
    }
  };
  try {
    std::thread my_thread(encode_and_save);
    my_thread.detach();
  } catch (std::exception &e) {
    std::cout << "exception" << std::endl;
    std::cout << e.what() << std::endl;
  }
  std::cout << "receive askDNhandling rpc!\n";

  return grpc::Status::OK;
}
bool ProxyImpl::init_coordinator() {
  std::string coordinator_ip_port = "localhost:50051";
  m_coordinator_stub =
      coordinator_proto::CoordinatorService::NewStub(grpc::CreateChannel(
          coordinator_ip_port, grpc::InsecureChannelCredentials()));
  return true;
}

} // namespace OppoProject
