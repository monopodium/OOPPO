#include "coordinator.h"
#include "tinyxml2.h"
#include <random>

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

  ObjectItemBigSmall new_object;

  /*编码参数*/
  int k = m_encode_parameter.k_datablock;
  int m = m_encode_parameter.g_m_globalparityblock;
  int l = m_encode_parameter.l_localgroup;
  new_object.object_size = valuesizebytes;

  /*文件分为三类：
  超大文件：value_size > blob_size_upper
  大文件：blob_size_upper >= value_size > small_file_upper
  小文件：small_file_upper >= value_size
  */

  /**meta data update*/
  if (valuesizebytes > m_encode_parameter.small_file_upper) {
    new_object.big_object = true;
    if (valuesizebytes > m_encode_parameter.blob_size_upper) {
      proxy_proto::ObjectAndPlacement object_placement;
      object_placement.set_bigobject(true);
      object_placement.set_key(key);
      object_placement.set_valuesizebyte(valuesizebytes);
      object_placement.set_k(k);
      object_placement.set_m(m);
      object_placement.set_l(l);
      int shard_size = ceil(m_encode_parameter.blob_size_upper, k);
      shard_size = 16 * ceil(shard_size, 16);
      object_placement.set_shard_size(shard_size);

      int num_of_stripes = valuesizebytes / (k * shard_size);
      for (int i = 0; i < num_of_stripes; i++) {
        valuesizebytes -= k * shard_size;
        StripeItem stripe;
        stripe.Stripe_id = m_next_stripe_id++;
        stripe.shard_size = shard_size;
        stripe.k = m_encode_parameter.k_datablock;
        stripe.l = m_encode_parameter.l_localgroup;
        stripe.g_m = m_encode_parameter.g_m_globalparityblock;
        // for (int i = 0; i < k + m; i++) {
        //   // 其实应该根据placement_plan来添加node_id
        //   stripe.nodes.push_back(i);
        // }
        generate_placement(stripe.nodes);
        new_object.stripes.push_back(stripe.Stripe_id);
        m_Stripe_info[stripe.Stripe_id] = stripe;

        object_placement.add_stripe_ids(stripe.Stripe_id);
        for (int i = 0; i < k + m; i++) {
          Nodeitem &node = m_Node_info[stripe.nodes[i]];
          object_placement.add_datanodeip(node.Node_ip.c_str());
          object_placement.add_datanodeport(node.Node_port);
        }
      }
      if (valuesizebytes > 0) {
        int shard_size = ceil(valuesizebytes, k);
        shard_size = 16 * ceil(shard_size, 16);
        StripeItem stripe;
        stripe.Stripe_id = m_next_stripe_id++;
        stripe.shard_size = shard_size;
        stripe.k = m_encode_parameter.k_datablock;
        stripe.l = m_encode_parameter.l_localgroup;
        stripe.g_m = m_encode_parameter.g_m_globalparityblock;
        // for (int i = 0; i < k + m; i++) {
        //   // 其实应该根据placement_plan来添加node_id
        //   stripe.nodes.push_back(i);
        // }
        generate_placement(stripe.nodes);
        new_object.stripes.push_back(stripe.Stripe_id);
        m_Stripe_info[stripe.Stripe_id] = stripe;

        object_placement.add_stripe_ids(stripe.Stripe_id);
        object_placement.set_tail_shard_size(shard_size);
        for (int i = 0; i < k + m; i++) {
          Nodeitem &node = m_Node_info[stripe.nodes[i]];
          object_placement.add_datanodeip(node.Node_ip.c_str());
          object_placement.add_datanodeport(node.Node_port);
        }
      } else {
        object_placement.set_tail_shard_size(-1);
      }
      grpc::ClientContext handle_ctx;
      proxy_proto::SetReply set_reply;
      grpc::Status status;
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<unsigned int> dis(0, m_AZ_info.size() - 1);
      std::string choose_proxy = m_AZ_info[dis(gen)].proxy_ip + ":" + std::to_string(m_AZ_info[dis(gen)].proxy_port);
      status = m_proxy_ptrs[choose_proxy]->EncodeAndSetObject(&handle_ctx, object_placement, &set_reply);
      std::string selected_proxy_ip = m_AZ_info[dis(gen)].proxy_ip;
      int selected_proxy_port = m_AZ_info[dis(gen)].proxy_port + 1;
      proxyIPPort->set_proxyip(selected_proxy_ip);
      proxyIPPort->set_proxyport(selected_proxy_port);
    } else {
      int shard_size = ceil(valuesizebytes, k);
      shard_size = 16 * ceil(shard_size, 16);
      StripeItem stripe;
      stripe.Stripe_id = m_next_stripe_id++;
      stripe.shard_size = shard_size;
      stripe.k = m_encode_parameter.k_datablock;
      stripe.l = m_encode_parameter.l_localgroup;
      stripe.g_m = m_encode_parameter.g_m_globalparityblock;
      // for (int i = 0; i < k + m; i++) {
      //   // 其实应该根据placement_plan来添加node_id
      //   stripe.nodes.push_back(i);
      // }
      generate_placement(stripe.nodes);
      new_object.stripes.push_back(stripe.Stripe_id);
      m_Stripe_info[stripe.Stripe_id] = stripe;

      grpc::ClientContext handle_ctx;
      proxy_proto::SetReply set_reply;
      grpc::Status status;
      proxy_proto::ObjectAndPlacement object_placement;

      object_placement.add_stripe_ids(stripe.Stripe_id);
      object_placement.set_bigobject(true);
      object_placement.set_key(key);
      object_placement.set_valuesizebyte(valuesizebytes);
      object_placement.set_k(k);
      object_placement.set_m(m);
      object_placement.set_l(l);
      object_placement.set_shard_size(shard_size);
      object_placement.set_tail_shard_size(-1);
      for (int i = 0; i < k + m; i++) {
        Nodeitem &node = m_Node_info[stripe.nodes[i]];
        object_placement.add_datanodeip(node.Node_ip.c_str());
        object_placement.add_datanodeport(node.Node_port);
      }

      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<unsigned int> dis(0, m_AZ_info.size() - 1);
      std::string choose_proxy = m_AZ_info[dis(gen)].proxy_ip + ":" + std::to_string(m_AZ_info[dis(gen)].proxy_port);
      status = m_proxy_ptrs[choose_proxy]->EncodeAndSetObject(&handle_ctx, object_placement, &set_reply);
      std::string selected_proxy_ip = m_AZ_info[dis(gen)].proxy_ip;
      int selected_proxy_port = m_AZ_info[dis(gen)].proxy_port + 1;
      proxyIPPort->set_proxyip(selected_proxy_ip);
      proxyIPPort->set_proxyport(selected_proxy_port);

      if (status.ok()) {

      } else {
        std::cout << "datanodes can not serve client download request!"
                  << std::endl;
        return grpc::Status::CANCELLED;
      }
    }
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
    int k = m_Stripe_info[object_infro.stripes[0]].k;
    int m = m_Stripe_info[object_infro.stripes[0]].g_m;

    grpc::ClientContext decode_and_get;
    proxy_proto::ObjectAndPlacement object_placement;
    grpc::Status status;
    proxy_proto::GetReply get_reply;
    getReplyClient->set_valuesizebytes(object_infro.object_size);
    if (object_infro.big_object) { /*大文件读*/
      object_placement.set_bigobject(true);
      object_placement.set_key(key);
      object_placement.set_valuesizebyte(object_infro.object_size);
      object_placement.set_k(k);
      object_placement.set_m(m);
      object_placement.set_tail_shard_size(-1);
      if (object_infro.object_size > m_encode_parameter.blob_size_upper) {
        int shard_size = ceil(m_encode_parameter.blob_size_upper, k);
        shard_size = 16 * ceil(shard_size, 16);
        object_placement.set_shard_size(shard_size);
        if (object_infro.object_size % (k * shard_size) != 0) {
          int tail_stripe_size = object_infro.object_size % (k * shard_size);
          int tail_shard_size = ceil(tail_stripe_size, k);
          tail_shard_size = 16 * ceil(tail_shard_size, 16);
          object_placement.set_tail_shard_size(tail_shard_size);
        }
      } else {
        int shard_size = ceil(object_infro.object_size, k);
        shard_size = 16 * ceil(shard_size, 16);
        object_placement.set_shard_size(shard_size);
      }

      for (int i = 0; i < object_infro.stripes.size(); i++) {
        StripeItem &stripe = m_Stripe_info[object_infro.stripes[i]];
        object_placement.add_stripe_ids(stripe.Stripe_id);
        for (int j = 0; j < stripe.nodes.size(); j++) {
          Nodeitem &node = m_Node_info[stripe.nodes[j]];
          object_placement.add_datanodeip(node.Node_ip.c_str());
          object_placement.add_datanodeport(node.Node_port);
        }
      }

      object_placement.set_clientip(client_ip);
      object_placement.set_clientport(client_port);

      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<unsigned int> dis(0, m_AZ_info.size() - 1);
      std::string choose_proxy = m_AZ_info[dis(gen)].proxy_ip + ":" + std::to_string(m_AZ_info[dis(gen)].proxy_port);
      status = m_proxy_ptrs[choose_proxy]->decodeAndGetObject(&decode_and_get, object_placement, &get_reply);
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
    std::string proxy_ip_and_port = cur->second.proxy_ip + ":" + std::to_string(cur->second.proxy_port);
    auto _stub = proxy_proto::proxyService::NewStub(grpc::CreateChannel(proxy_ip_and_port, grpc::InsecureChannelCredentials()));
    m_proxy_ptrs.insert(std::make_pair(proxy_ip_and_port, std::move(_stub)));
  }


  std::string proxy_ip_port = "0.0.0.0:50055";
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
    auto pos = proxy.find(':');
    m_AZ_info[std::stoi(az_id)].proxy_ip = proxy.substr(0, pos);
    m_AZ_info[std::stoi(az_id)].proxy_port = std::stoi(proxy.substr(pos+1, proxy.size()));
    for (tinyxml2::XMLElement* node = az->FirstChildElement()->FirstChildElement(); node != nullptr; node = node->NextSiblingElement()) {
      std::string node_uri(node->Attribute("uri"));
      std::cout << "____node: " << node_uri << std::endl;
      m_AZ_info[std::stoi(az_id)].nodes.push_back(node_id);
      m_Node_info[node_id].Node_id = node_id;
      auto pos = node_uri.find(':');
      m_Node_info[node_id].Node_ip = node_uri.substr(0, pos);
      m_Node_info[node_id].Node_port = std::stoi(node_uri.substr(pos+1, node_uri.size()));
      m_Node_info[node_id].AZ_id = std::stoi(az_id);
      node_id++;
    }
  }
}
void CoordinatorImpl::generate_placement(std::vector<unsigned int> &stripe_nodes) {
  // Flat以后再说

  int k = m_encode_parameter.k_datablock;
  int l = m_encode_parameter.l_localgroup;
  int g_m = m_encode_parameter.g_m_globalparityblock;
  // 假设k被l整除
  int b = k / l;
  OppoProject::EncodeType encode_type = m_encode_parameter.encodetype;
  OppoProject::PlacementType placement_type = m_encode_parameter.placementtype;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<unsigned int> dis(0, m_Node_info.size() - 1);

  if (encode_type == RS) {
    // RS只能使用Flat或者Random
    // stripe_nodes依次对应k个数据块、g个校验块的放置节点
    if (placement_type == Random) {
      std::vector<bool> vis(m_Node_info.size(), false);
      std::vector<int> num_chosen_nodes_per_az(m_AZ_info.size(), 0);
      for (int i = 0; i < k + g_m; i++) {
        int node_idx;
        do {
          node_idx = dis(gen);
          // 每个AZ内不能放置超过k个块，以保证单AZ可修复
        } while(vis[node_idx] == true || num_chosen_nodes_per_az[m_Node_info[node_idx].AZ_id] == k);
        stripe_nodes.push_back(node_idx);
        vis[node_idx] = true;
        num_chosen_nodes_per_az[m_Node_info[node_idx].AZ_id]++;
      }
    }
  } else if (encode_type == Azure_LRC_1) {
    // stripe_nodes依次对应k个数据块、g个全局校验块、l+1个局部校验块的放置节点
    // 且k个数据块被分为了l个组，每个组在stripe_nodes中是连续的
    if (placement_type == Random) {
      std::vector<bool> vis(m_Node_info.size(), false);
      std::vector<std::pair<std::unordered_set<int>, int>> help(m_AZ_info.size());
      for (int i = 0; i < m_AZ_info.size(); i++) {
        help[i].second = 0;
      }
      int node_idx, az_idx, area_upper;
      for (int i = 0; i < l; i++) {
        for (int j = 0; j < b; j++) {
          do {
            node_idx = dis(gen);
            az_idx = m_Node_info[node_idx].AZ_id;
            area_upper = g_m + help[az_idx].first.size();
          } while (vis[node_idx] == true || help[az_idx].second == area_upper);
          stripe_nodes.push_back(node_idx);
          vis[node_idx] = true;
          help[az_idx].first.insert(i);
          help[az_idx].second++;
        }
      }
      for (int i = 0; i < g_m; i++) {
        do {
          node_idx = dis(gen);
          az_idx = m_Node_info[node_idx].AZ_id;
          area_upper = g_m + help[az_idx].first.size();
        } while (vis[node_idx] == true || help[az_idx].second == area_upper);
        stripe_nodes.push_back(node_idx);
        vis[node_idx] = true;
        help[az_idx].second++;
      }
      for (int i = 0; i < l; i++) {
        do {
          node_idx = dis(gen);
          az_idx = m_Node_info[node_idx].AZ_id;
          area_upper = g_m + help[az_idx].first.size();
        } while (vis[node_idx] == true || help[az_idx].second == area_upper);
        stripe_nodes.push_back(node_idx);
        vis[node_idx] = true;
        if (help[az_idx].first.count(i) == 0) {
          help[az_idx].first.insert(i);
        }
        help[az_idx].second++;
      }
      // 最后还有1个由全局校验块生成的局部校验块
      do {
        node_idx = dis(gen);
        az_idx = m_Node_info[node_idx].AZ_id;
        area_upper = g_m + help[az_idx].first.size();
      } while (vis[node_idx] == true || help[az_idx].second == area_upper);
      stripe_nodes.push_back(node_idx);
      vis[node_idx] = true;
      help[az_idx].second++;
    } else if (placement_type == Best_Placement) {
      int start_idx = 0;
      int sita = g_m / b;
      stripe_nodes.resize(k + g_m + l + 1);
      if (sita >= 1) {
        int left_data_shard = k;
        while (left_data_shard > 0) {
          if (left_data_shard >= sita * b) {
            cur_az = cur_az % m_AZ_info.size();
            AZitem &az = m_AZ_info[cur_az++];
            for (int i = 0; i < sita * b; i++) {
              cur_node = cur_node % az.nodes.size();
              stripe_nodes[start_idx + i] = az.nodes[cur_node++];
            }
            for (int i = 0; i < sita; i++) {
              cur_node = cur_node % az.nodes.size();
              stripe_nodes[k + g_m + start_idx / b + i] = az.nodes[cur_node++];
            }
            start_idx += (sita * b);
            left_data_shard -= (sita * b);
          } else {
            int left_group = left_data_shard / b;
            cur_az = cur_az % m_AZ_info.size();
            AZitem &az = m_AZ_info[cur_az++];
            for (int i = 0; i < left_data_shard; i++) {
              cur_node = cur_node % az.nodes.size();
              stripe_nodes[start_idx + i] = az.nodes[cur_node++];
            }
            for (int i = 0; i < left_group; i++) {
              cur_node = cur_node % az.nodes.size();
              stripe_nodes[k + g_m + start_idx / b + i] = az.nodes[cur_node++];
            }
            start_idx += (left_data_shard);
            left_data_shard -= left_data_shard;
          }
        }
        cur_az = cur_az % m_AZ_info.size();
        AZitem &az = m_AZ_info[cur_az++];
        for (int i = 0; i < g_m; i++) {
          cur_node = cur_node % az.nodes.size();
          stripe_nodes[k + i] = az.nodes[cur_node++];
        }
        cur_node = cur_node % az.nodes.size();
        stripe_nodes[stripe_nodes.size() - 1] = az.nodes[cur_node++];
      } else {
        int idx = 0;
        for (int i = 0; i < l; i++) {
          int left_data_shard_in_group = b;
          while (left_data_shard_in_group >= g_m + 1) {
            cur_az = cur_az % m_AZ_info.size();
            AZitem &az = m_AZ_info[cur_az++];
            for (int j = 0; j < g_m + 1; j++) {
              cur_node = cur_node % az.nodes.size();
              stripe_nodes[idx++] = az.nodes[cur_node++];
            }
            if (left_data_shard_in_group == g_m + 1) {
              cur_node = cur_node % az.nodes.size();
              stripe_nodes[k + g_m + i] = az.nodes[cur_node++];
            }
            left_data_shard_in_group -= (g_m + 1);
          }
          cur_az = cur_az % m_AZ_info.size();
          AZitem &az = m_AZ_info[cur_az++];
          for (int i = 0; i < left_data_shard_in_group; i++) {
            cur_node = cur_node % az.nodes.size();
            stripe_nodes[idx++] = az.nodes[cur_node++];
          }
        }
        cur_az = cur_az % m_AZ_info.size();
        AZitem &az = m_AZ_info[cur_az++];
        for (int i = 0; i < g_m; i++) {
          cur_node = cur_node % az.nodes.size();
          stripe_nodes[k + i] = az.nodes[cur_node++];
        }
        cur_node = cur_node % az.nodes.size();
        stripe_nodes[stripe_nodes.size() - 1] = az.nodes[cur_node++];
      }
    }
  } else if (encode_type == OPPO_LRC) {
    // OPPO_LRC1个group放1个az
    stripe_nodes.resize(k + g_m + l + 1);
    int idx = 0;
    for (int i = 0; i < l; i++) {
      cur_az = cur_az % m_AZ_info.size();
      AZitem &az = m_AZ_info[cur_az++];
      for (int j = 0; j < b; j++) {
        cur_node = cur_node % az.nodes.size();
        stripe_nodes[idx++] = az.nodes[cur_node++];
      }
      cur_node = cur_node % az.nodes.size();
      stripe_nodes[k + g_m + i] = az.nodes[cur_node++];
    }
    cur_az = cur_az % m_AZ_info.size();
    AZitem &az = m_AZ_info[cur_az++];
    for (int i = 0; i < g_m; i++) {
      cur_node = cur_node % az.nodes.size();
      stripe_nodes[k + i] = az.nodes[cur_node++];
    }
    cur_node = cur_node % az.nodes.size();
    stripe_nodes[stripe_nodes.size() - 1] = az.nodes[cur_node++];
  }
  return;
}

} // namespace OppoProject