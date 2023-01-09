#include "coordinator.h"
#include "tinyxml2.h"

namespace OppoProject {

template <typename T> inline T ceil(T const &A, T const &B) {
  return T((A + B - 1) / B);
};

grpc::Status CoordinatorImpl::setParameter(
    ::grpc::ServerContext *context,
    const coordinator_proto::Parameter *parameter,
    coordinator_proto::RepIfSetParaSucess *setParameterReply) {
  ECSchema system_metadata(parameter->partial_decoding(),
                           (OppoProject::EncodeType)parameter->encodetype(),
                           (OppoProject::PlacementType)parameter->placementtype(),
                           parameter->k_datablock(),
                           parameter->l_localgroup(),
                           parameter->g_m_globalparityblock(),
                           parameter->r_datapergoup(),
                           parameter->small_file_upper(),
                           parameter->blob_size_upper());
  m_encode_parameter = system_metadata;
  setParameterReply->set_ifsetparameter(true);
  std::cout << "setParameter success" << std::endl;
  return grpc::Status::OK;
}

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
  int k = m_encode_parameter.k_datablock;
  int m = m_encode_parameter.g_m_globalparityblock;
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

      std::vector<std::pair<std::string, int>> datanodeip_port;

      generate_placement(datanodeip_port);
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
      new_object.block_size = block_size;
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
  std::lock_guard<std::mutex> lck(m_mutex);
  m_object_table_big_small_updating[key] = new_object;

  /*inform proxy*/

  return grpc::Status::OK;
}

grpc::Status
CoordinatorImpl::getValue(::grpc::ServerContext *context,
                          const coordinator_proto::KeyAndClientIP *keyClient,
                          coordinator_proto::RepIfGetSucess *getReplyClient) {
  try {
    std::lock_guard<std::mutex> lck(m_mutex);
    std::string key = keyClient->key();
    std::string client_ip = keyClient->clientip();
    int client_port = keyClient->clientport();
    for (auto it = m_object_table_big_small_commit.cbegin();
         it != m_object_table_big_small_commit.cend(); it++) {
      std::cout << "it->first:" << it->first << std::endl;
    }
    ObjectItemBigSmall object_infro = m_object_table_big_small_commit.at(key);

    grpc::ClientContext decode_and_get;
    proxy_proto::ObjectAndPlacement object_placement;
    grpc::Status status;
    proxy_proto::GetReply get_reply;
    getReplyClient->set_valuesizebytes(object_infro.object_size);
    if (object_infro.big_object) { /*大文件读*/
      object_placement.set_key(key);
      object_placement.set_valuesizebyte(object_infro.object_size);
      object_placement.set_k(m_encode_parameter.k_datablock);
      object_placement.set_m(m_encode_parameter.g_m_globalparityblock);
      object_placement.set_blocksizebyte(object_infro.block_size);

      for (auto it = object_infro.shard_id.cbegin();
           it < object_infro.shard_id.cend(); it++) {
        object_placement.add_shardid(*(it));
      }
      object_placement.set_clientip(client_ip);
      object_placement.set_clientport(client_port);

      status = m_proxy_ptrs["localhost:50055"]->decodeAndGetObject(
          &decode_and_get, object_placement, &get_reply);
    } else {
    }
  } catch (std::exception &e) {
    std::cout << "exception" << std::endl;
    std::cout << e.what() << std::endl;
  }
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
  std::lock_guard<std::mutex> lck(m_mutex);
  try {
    if (commit_abortkey->ifcommitmetadata()) {
      std::pair<std::string, ObjectItemBigSmall> myshopping(
          key, m_object_table_big_small_updating[key]);
      m_object_table_big_small_commit.insert(myshopping);

      m_object_table_big_small_updating.erase(key);
      std::cout << "m_object_table_big_small_commit.at(key).object_size  "
                << m_object_table_big_small_commit.at(key).object_size
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

grpc::Status
CoordinatorImpl::checkCommitAbort(grpc::ServerContext *context,
                                  const coordinator_proto::AskIfSetSucess *key,
                                  coordinator_proto::RepIfSetSucess *reply) {
  while (m_object_table_big_small_commit.find(key->key()) ==
         m_object_table_big_small_commit.end()) {
    // std::cout << "waiting key!" << std::endl;
  }
  reply->set_ifcommit(true);
  /*待补充*/
  return grpc::Status::OK;
}
bool CoordinatorImpl::init_proxy(std::string proxy_information_path) {
  /*需要补充修改，这里需要读取.xml的proxy的ip来初始化，
  将proxy的_stub初始化到m_proxy_ptrs中*/
  /*配置文件的路径是proxy_information_path*/
  
  /*没必要再读取配置文件了*/
  for (auto cur = m_AZ_info.begin(); cur != m_AZ_info.end(); cur++) {
    auto _stub = proxy_proto::proxyService::NewStub(grpc::CreateChannel(cur->second.proxy, grpc::InsecureChannelCredentials()));
    m_proxy_ptrs.insert(std::make_pair(cur->second.proxy, std::move(_stub)));
  }


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
bool CoordinatorImpl::init_AZinformation(std::string Azinformation_path) {

  /*需要补充修改，这里需要读取.xml的proxy的ip来初始化，
将datanode和AZ的信息初始化到m_AZ_info中*/
  /*配置文件的路径是Azinformation_path*/
  std::cout << "Azinformation_path:" << Azinformation_path << std::endl;
  tinyxml2::XMLDocument xml;
  xml.LoadFile(Azinformation_path.c_str());
  tinyxml2::XMLElement *root = xml.RootElement();
  int node_id = 0;
  for (tinyxml2::XMLElement* az = root->FirstChildElement(); az != nullptr; az = az->NextSiblingElement()) {
    std::string az_id(az->Attribute("id"));
    std::string proxy(az->Attribute("proxy"));
    std::cout << "az_id: " << az_id << " , proxy: " << proxy << std::endl;
    m_AZ_info[std::stoi(az_id)].AZ_id = std::stoi(az_id);
    m_AZ_info[std::stoi(az_id)].proxy = proxy;
    for (tinyxml2::XMLElement* node = az->FirstChildElement()->FirstChildElement(); node != nullptr; node = node->NextSiblingElement()) {
      std::string node_uri(node->Attribute("uri"));
      std::cout << "____node: " << node_uri << std::endl;
      m_AZ_info[std::stoi(az_id)].DataNodeInfo.push_back(node_id);
      m_Node_info[node_id].Node_id = node_id;
      m_Node_info[node_id].ip_port = node_uri;
      m_Node_info[node_id].AZ_id = std::stoi(az_id);
    }
  }
}
void CoordinatorImpl::generate_placement(
    std::vector<std::pair<std::string, int>> datanodeip_port) {
  /*根据m_encode_parameter中生成的编码信息,为一个stripe生成放置策略，
  然后把生成的放置策略按照k个数据块的Ip,l个局部校验块的Ip,g个全局校验块的ip排列，
  要是是RS,就k个，m个*/
}

} // namespace OppoProject