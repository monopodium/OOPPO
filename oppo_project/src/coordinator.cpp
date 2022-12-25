#include "coordinator.h"
#include "devcommon.h"

namespace OppoProject {

template <typename T> inline T ceil(T const &A, T const &B) {
  return T((A + B - 1) / B);
};
grpc::Status CoordinatorImpl::sayHelloToCoordinator(
    ::grpc::ServerContext *context,
    const coordinator_proto::RequestToCoordinator *helloRequestToCoordinator,
    coordinator_proto::ReplyFromCoordinator *helloReplyFromCoordinator) {
  std::string prefix("Hello ");
  helloReplyFromCoordinator->set_message(prefix +
                                         helloRequestToCoordinator->name());
  std::cout << prefix + helloRequestToCoordinator->name() << std::endl;
  return grpc::Status::OK;
}

grpc::Status CoordinatorImpl::uploadOriginKeyValue(
    ::grpc::ServerContext *context,
    const coordinator_proto::RequestProxyIPPort *keyValueSize,
    coordinator_proto::ReplyProxyIPPort *proxyIPPort) {

  std::string key = keyValueSize->key();
  int valuesizebytes = keyValueSize->valuesizebytes();
  std::string selected_proxy_ip = "localhost";
  short selected_proxy_port = 12233;
  proxyIPPort->set_proxyip(selected_proxy_ip);
  proxyIPPort->set_proxyport(selected_proxy_port);

  ObjectItemBigSmall new_object;

  /*编码参数*/
  int k = 3;
  int m = 2;
  new_object.object_size = valuesizebytes;

  /*大文件分为三类：
  超大，中大
  超大文件跨stripe valuesizebytes > max_stripe_size (max_stripe_size =
  k*SHARD_SIZE_UPPER_BOUND_BYTE) 中大文件一个stripe 每个stipe的大小不一定喔
  */

  /**meta data update*/
  if (valuesizebytes > BOUNDARY_BIG_SMALL_OBJECT_BYTE) {
    new_object.big_object = true;
    if (valuesizebytes > k * SHARD_SIZE_UPPER_BOUND_BYTE) {

    } else {

      for (int i = 0; i < k; i++) {
        int shard_id = m_next_stripe_id * 1000 + i;
        new_object.shard_id.push_back(shard_id);
      }
      for (int i = 0; i < m; i++) {
        int shard_id = m_next_stripe_id * 1000 + i + k;
        new_object.shard_id.push_back(shard_id);
      }
      /**generate placement and encode schema*/
      /**以后把这部分替换成函数哦*/

      int block_size = ceil(valuesizebytes, k);
      block_size = 16 * ceil(block_size, 16);
      grpc::ClientContext handle_ctx;
      proxy_proto::SetReply set_reply;
      grpc::Status status;
      proxy_proto::ObjectAndPlacement object_placement;

      object_placement.set_bigobject(true);
      object_placement.set_key(key);
      object_placement.set_valuesizebyte(valuesizebytes);
      object_placement.set_k(k);
      object_placement.set_m(m);
      object_placement.set_blocksizebyte(block_size);
      for (auto it = new_object.shard_id.begin();
           it != new_object.shard_id.end(); it++) {
        object_placement.add_shardid(*it);
      }

      object_placement.add_datanodeip("localhost");
      object_placement.add_datanodeport(11111);
      object_placement.add_datanodeip("localhost");
      object_placement.add_datanodeport(11112);
      object_placement.add_datanodeip("localhost");
      object_placement.add_datanodeport(11113);
      object_placement.add_datanodeip("localhost");
      object_placement.add_datanodeport(11114);
      object_placement.add_datanodeip("localhost");
      object_placement.add_datanodeport(11115);
      status = m_proxy_ptrs["localhost:50055"]->EncodeAndSetObject(
          &handle_ctx, object_placement, &set_reply);

      if (status.ok()) {

      } else {
        std::cout << "datanodes can not serve client download request!"
                  << std::endl;
        return grpc::Status::CANCELLED;
      }
    }
    m_next_stripe_id++;
  } else {
    // Ayuan
  }
  m_object_table_big_small_updating[key] = new_object;

  /*inform proxy*/

  return grpc::Status::OK;
}
grpc::Status CoordinatorImpl::checkalive(
    grpc::ServerContext *context,
    const coordinator_proto::RequestToCoordinator *helloRequestToCoordinator,
    coordinator_proto::ReplyFromCoordinator *helloReplyFromCoordinator) {

  std::cout << "checkalive" << helloRequestToCoordinator->name() << std::endl;
  return grpc::Status::OK;
}
grpc::Status CoordinatorImpl::reportCommitAbort(
    grpc::ServerContext *context,
    const coordinator_proto::CommitAbortKey *commit_abortkey,
    coordinator_proto::ReplyFromCoordinator *helloReplyFromCoordinator) {
  std::string key = commit_abortkey->key();
  try {
    if (commit_abortkey->ifcommitmetadata()) {
      std::pair<std::string, ObjectItemBigSmall> myshopping(
          key, m_object_table_big_small_updating[key]);
      object_table_big_small_commit.insert(myshopping);

      m_object_table_big_small_updating.erase(key);
      std::cout << "object_table_big_small_commit.at(key).object_size  "
                << object_table_big_small_commit.at(key).object_size
                << std::endl;

    } else {
      m_object_table_big_small_updating.erase(key);
    }
  } catch (std::exception &e) {
    std::cout << "exception" << std::endl;
    std::cout << e.what() << std::endl;
  }
  return grpc::Status::OK;
}
} // namespace OppoProject