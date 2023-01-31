#include "coordinator.h"
#include "tinyxml2.h"
#include <random>
#include <thread>

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
      object_placement.set_encode_type((int)m_encode_parameter.encodetype);
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
        generate_placement(stripe.nodes, stripe.Stripe_id);
        new_object.stripes.push_back(stripe.Stripe_id);
        m_Stripe_info[stripe.Stripe_id] = stripe;

        object_placement.add_stripe_ids(stripe.Stripe_id);
        for (int i = 0; i < stripe.nodes.size(); i++) {
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
        generate_placement(stripe.nodes, stripe.Stripe_id);
        new_object.stripes.push_back(stripe.Stripe_id);
        m_Stripe_info[stripe.Stripe_id] = stripe;

        object_placement.add_stripe_ids(stripe.Stripe_id);
        object_placement.set_tail_shard_size(shard_size);
        for (int i = 0; i < stripe.nodes.size(); i++) {
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
      generate_placement(stripe.nodes, stripe.Stripe_id);
      new_object.stripes.push_back(stripe.Stripe_id);
      m_Stripe_info[stripe.Stripe_id] = stripe;

      grpc::ClientContext handle_ctx;
      proxy_proto::SetReply set_reply;
      grpc::Status status;
      proxy_proto::ObjectAndPlacement object_placement;

      object_placement.set_encode_type((int)m_encode_parameter.encodetype);
      object_placement.add_stripe_ids(stripe.Stripe_id);
      object_placement.set_bigobject(true);
      object_placement.set_key(key);
      object_placement.set_valuesizebyte(valuesizebytes);
      object_placement.set_k(k);
      object_placement.set_m(m);
      object_placement.set_l(l);
      object_placement.set_shard_size(shard_size);
      object_placement.set_tail_shard_size(-1);
      for (int i = 0; i < stripe.nodes.size(); i++) {
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
    int l = m_Stripe_info[object_infro.stripes[0]].l;


    grpc::ClientContext decode_and_get;
    proxy_proto::ObjectAndPlacement object_placement;
    object_placement.set_encode_type((int)m_encode_parameter.encodetype);
    grpc::Status status;
    proxy_proto::GetReply get_reply;
    getReplyClient->set_valuesizebytes(object_infro.object_size);
    if (object_infro.big_object) { /*大文件读*/
      object_placement.set_encode_type((int)m_encode_parameter.encodetype);
      object_placement.set_bigobject(true);
      object_placement.set_key(key);
      object_placement.set_valuesizebyte(object_infro.object_size);
      object_placement.set_k(k);
      object_placement.set_m(m);
      object_placement.set_l(l);
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

grpc::Status CoordinatorImpl::requestRepair(::grpc::ServerContext *context,
                                            const coordinator_proto::FailNodes *failed_node_list,
                                            coordinator_proto::RepIfRepairSucess *reply) {
  std::vector<int> failed_node_ids;
  for (int i = 0; i < failed_node_list->node_list_size(); i++) {
    std::string node = failed_node_list->node_list(i);
    for (auto &node_info : m_Node_info) {
      // 这里其实应该用ip来判断
      // 由于我们现在是在单机上进行测试，所以暂时用端口号代替
      if (node_info.second.Node_port == std::stoi(node)) {
        failed_node_ids.push_back(node_info.second.Node_id);
        break;
      }
    }
  }
  std::unordered_set<int> failed_stripe_ids;
  for (auto &node_id : failed_node_ids) {
    for (auto &stripe_id : m_Node_info[node_id].stripes) {
      failed_stripe_ids.insert(stripe_id);
    }
  }
  for (auto &stripe_id : failed_stripe_ids) {
    std::vector<int> failed_shard_idxs;
    for (size_t i = 0; i < m_Stripe_info[stripe_id].nodes.size(); i++) {
      auto lookup = std::find(failed_node_ids.begin(), failed_node_ids.end(), m_Stripe_info[stripe_id].nodes[i]);
      if (lookup != failed_node_ids.end()) {
        failed_shard_idxs.push_back(i);
      }
    }
    do_repair(stripe_id, failed_shard_idxs);
  }
  reply->set_ifrepair(true);
  return grpc::Status::OK;
}

bool num_survive_nodes(std::pair<int, std::vector<std::pair<int, int>>> &a, std::pair<int, std::vector<std::pair<int, int>>> &b) {
  return a.second.size() > b.second.size();
}

void CoordinatorImpl::generate_repair_plan(int stripe_id, bool one_shard, std::vector<int> &failed_shard_idxs, std::vector<std::vector<std::pair<std::string, std::string>>> &shards_to_read, std::vector<int> &repair_span_az) {
  int k = m_encode_parameter.k_datablock;
  int l = m_encode_parameter.l_localgroup;
  int g = m_encode_parameter.g_m_globalparityblock;
  int b = k / l;
  int tail_group = k - l * b;
  int real_l = tail_group > 0 ? (l + 1) : (l);
  StripeItem &stripe_info = m_Stripe_info[stripe_id];
  if (one_shard) {
    int failed_shard_idx = failed_shard_idxs[0];
    Nodeitem &failed_node_info = m_Node_info[stripe_info.nodes[failed_shard_idx]];
    int main_az_id = failed_node_info.AZ_id;
    repair_span_az.push_back(main_az_id);
    if (m_encode_parameter.encodetype == RS) {
      std::vector<std::pair<std::string, std::string>> temp1;
      for (auto &node_id : m_AZ_info[main_az_id].nodes) {
        for (size_t i = 0; i < stripe_info.nodes.size(); i++) {
          // 检查该node是否属于损坏的stripe，且不能是损坏的node
          if (node_id == stripe_info.nodes[i] && node_id != failed_node_info.Node_id) {
            std::string shard_location = m_Node_info[node_id].Node_ip + ":" + std::to_string(m_Node_info[node_id].Node_port);
            std::string shard_id = std::to_string(stripe_id * 1000 + i);
            temp1.push_back({shard_location, shard_id});
            break;
          }
        }
        if (temp1.size() == k) {
          shards_to_read.push_back(temp1);
          goto finish;
        }
      }
      shards_to_read.push_back(temp1);
      int more_shards = k - temp1.size();
      std::vector<std::pair<int, std::vector<std::pair<int, int>>>> help;
      for (auto &az_pair : m_AZ_info) {
        if (az_pair.first != main_az_id) {
          std::vector<std::pair<int, int>> nodes_id_and_idx;
          for (auto &node_id : az_pair.second.nodes) {
            // 检查该node是不是属于损坏的stripe的
            for (size_t i = 0; i < stripe_info.nodes.size(); i++) {
              if (node_id == stripe_info.nodes[i]) {
                nodes_id_and_idx.push_back({node_id, i});
              }
            }
          }
          help.push_back({az_pair.first, nodes_id_and_idx});
        }
      }
      // 尽量选择存活块更多的help az
      sort(help.begin(), help.end(), num_survive_nodes);
      int sum = 0;
      for (auto &help_az : help) {
        repair_span_az.push_back(help_az.first);
        std::vector<std::pair<std::string, std::string>> temp2;
        for (auto &node_id_and_idx : help_az.second) {
          std::string shard_location = m_Node_info[node_id_and_idx.first].Node_ip + ":" + std::to_string(m_Node_info[node_id_and_idx.first].Node_port);
          std::string shard_id = std::to_string(stripe_id * 1000 + node_id_and_idx.second);
          temp2.push_back({shard_location, shard_id});
          if ((sum + temp2.size()) == more_shards) {
            shards_to_read.push_back(temp2);
            sum += temp2.size();
            goto finish;
          }
        }
        shards_to_read.push_back(temp2);
        sum += temp2.size();
      }
    } else if (m_encode_parameter.encodetype == Azure_LRC_1) {
      if (failed_shard_idx == (k + g + real_l) || (failed_shard_idx >= k && failed_shard_idx <= (k + g - 1))) {
         // 全局校验块那个组的块损坏，一定处于同一个AZ
         std::vector<std::pair<std::string, std::string>> temp3;
         for (int i = k; i <= (k + g - 1); i++) {
          if (i != failed_shard_idx) {
            Nodeitem &node_info = m_Node_info[stripe_info.nodes[i]];
            std::string shard_location = node_info.Node_ip + ":" + std::to_string(node_info.Node_port);
            std::string shard_id = std::to_string(stripe_id * 1000 + i);
            temp3.push_back({shard_location, shard_id});
          }
         }
         if (failed_shard_idx != (k + g + real_l)) {
          Nodeitem &node_info = m_Node_info[stripe_info.nodes[k + g + real_l]];
          std::string shard_location = node_info.Node_ip + ":" + std::to_string(node_info.Node_port);
          std::string shard_id = std::to_string(stripe_id * 1000 + k + g + real_l);
          temp3.push_back({shard_location, shard_id});
         }
         shards_to_read.push_back(temp3);
      } else {
        // 数据块所在的组损坏，可能波及多个AZ
        int group_idx;
        if (failed_shard_idx >= 0 && failed_shard_idx <= (k - 1)) {
          group_idx = failed_shard_idx / b;
        } else {
          group_idx = failed_shard_idx - (k + g);
        }
        std::vector<std::pair<int, int>> live_shards_in_group;
        for (int i = 0; i < b; i++) {
          int idx = group_idx * b + i;
          if (idx == failed_shard_idx) {
            continue;
          }
          if (idx >= k) {
            break;
          }
          live_shards_in_group.push_back({stripe_info.nodes[idx], idx});
        }
        if (k + g + group_idx != failed_shard_idx) {
          live_shards_in_group.push_back({stripe_info.nodes[k + g + group_idx], k + g + group_idx});
        }
        std::unordered_set<int> live_shards_group_span_az;
        for (auto &shard : live_shards_in_group) {
          live_shards_group_span_az.insert(m_Node_info[shard.first].AZ_id);
        }
        for (auto &az_id : live_shards_group_span_az) {
          if (az_id != main_az_id) {
            repair_span_az.push_back(az_id);
          }
        }
        for (auto &az_id : repair_span_az) {
          std::vector<std::pair<std::string, std::string>> temp4;
          for (auto &live_shard : live_shards_in_group) {
            Nodeitem &node_info = m_Node_info[live_shard.first];
            if (node_info.AZ_id == az_id) {
              std::string shard_location = node_info.Node_ip + ":" + std::to_string(node_info.Node_port);
              std::string shard_id = std::to_string(stripe_id * 1000 + live_shard.second);
              temp4.push_back({shard_location, shard_id});
            }
          }
          shards_to_read.push_back(temp4);
        }
      }
    } else if (m_encode_parameter.encodetype == OPPO_LRC) {
      // OPPO_LRC单块错误只会出现在1个AZ中，但这牺牲了容错性，在某些参数下无法保证单AZ故障可修复
      std::vector<std::pair<std::string, std::string>> temp5;
      int group_idx;
      std::vector<int> g_num_per_az(real_l, 0);
      int idx = 0;
      for (int i = 0; i < g; i++) {
        idx = idx % g_num_per_az.size();
        g_num_per_az[idx++]++;
      }
      if (failed_shard_idx >= 0 && failed_shard_idx <= (k - 1)) {
        group_idx = failed_shard_idx / b;
      } else if (failed_shard_idx >= k && failed_shard_idx <= (k + g - 1)) {
        int sum = 0;
        for (int i = 0; i < real_l; i++) {
          if (k + sum + g_num_per_az[i] - 1 >= failed_shard_idx) {
            group_idx = i;
            break;
          }
          sum += g_num_per_az[i];
        }
      } else {
        group_idx = failed_shard_idx - (k + g);
      }
      for (int i = 0; i < b; i++) {
        int idx = group_idx * b + i;
        if (idx == failed_shard_idx) {
          continue;
        }
        if (idx >= k) {
          break;
        }
        Nodeitem &node_info = m_Node_info[stripe_info.nodes[idx]];
        std::string shard_location = node_info.Node_ip + ":" + std::to_string(node_info.Node_port);
        std::string shard_id = std::to_string(stripe_id * 1000 + idx);
        temp5.push_back({shard_location, shard_id});
      }
      int g_start = 0;
      g_start += k;
      for (int i = 0; i < group_idx; i++) {
        g_start += g_num_per_az[i];
      }
      for (int j = 0, i = g_start; j < g_num_per_az[group_idx]; j++, i++) {
        if (i == failed_shard_idx) {
          continue;
        }
        Nodeitem &node_info = m_Node_info[stripe_info.nodes[i]];
        std::string shard_location = node_info.Node_ip + ":" + std::to_string(node_info.Node_port);
        std::string shard_id = std::to_string(stripe_id * 1000 + idx);
        temp5.push_back({shard_location, shard_id});
      }
      if (k + g + group_idx != failed_shard_idx) {
        Nodeitem &node_info = m_Node_info[stripe_info.nodes[k + g + group_idx]];
        std::string shard_location = node_info.Node_ip + ":" + std::to_string(node_info.Node_port);
        std::string shard_id = std::to_string(stripe_id * 1000 + idx);
        temp5.push_back({shard_location, shard_id});
      }
      shards_to_read.push_back(temp5);
    }
  }
finish:
  return;
}

void CoordinatorImpl::do_repair(int stripe_id, std::vector<int> failed_shard_idxs) {
  int k = m_encode_parameter.k_datablock;
  int l = m_encode_parameter.l_localgroup;
  int g = m_encode_parameter.g_m_globalparityblock;
  int b = k / l;
  int tail_group = k - l * b;
  if (failed_shard_idxs.size() == 1) {
    // 以下两个变量指示了修复流程涉及的AZ以及需要从每个AZ中读取的块
    // 第1个az是main az
    std::vector<std::vector<std::pair<std::string, std::string>>> shards_to_read;
    std::vector<int> repair_span_az;
    generate_repair_plan(stripe_id, true, failed_shard_idxs, shards_to_read, repair_span_az);
    int main_az_id = repair_span_az[0];
    bool multi_az = (repair_span_az.size() > 1);

    std::vector<std::thread> repairs;
    for (auto &az_id : repair_span_az) {
      if (az_id == main_az_id) {
        repairs.push_back(std::thread([&](){
          std::string main_ip_port = m_AZ_info[main_az_id].proxy_ip + ":" + std::to_string(m_AZ_info[main_az_id].proxy_port);
          grpc::ClientContext context;
          proxy_proto::mainRepairPlan request;
          proxy_proto::mainRepairReply reply;
          request.set_one_shard_fail(true);
          request.set_multi_az(multi_az);

          m_proxy_ptrs[main_ip_port]->mainRepair(&context, request, &reply);
        }));
      } else {

      }
    }

  } else {

  }

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
void CoordinatorImpl::generate_placement(std::vector<unsigned int> &stripe_nodes, int stripe_id) {
  // Flat以后再说

  int k = m_encode_parameter.k_datablock;
  int l = m_encode_parameter.l_localgroup;
  int g_m = m_encode_parameter.g_m_globalparityblock;
  int b = k / l;
  int tail_group = k - l * b;
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
        m_Node_info[node_idx].stripes.insert(stripe_id);
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
          m_Node_info[node_idx].stripes.insert(stripe_id);
          vis[node_idx] = true;
          help[az_idx].first.insert(i);
          help[az_idx].second++;
        }
      }
      for (int i = 0; i < tail_group; i++) {
        do {
          node_idx = dis(gen);
          az_idx = m_Node_info[node_idx].AZ_id;
          area_upper = g_m + help[az_idx].first.size();
        } while(vis[node_idx] == true || help[az_idx].second == area_upper);
        stripe_nodes.push_back(node_idx);
        m_Node_info[node_idx].stripes.insert(stripe_id);
        vis[node_idx] = true;
        help[az_idx].first.insert(l);
        help[az_idx].second++;
      }
      for (int i = 0; i < g_m; i++) {
        do {
          node_idx = dis(gen);
          az_idx = m_Node_info[node_idx].AZ_id;
          area_upper = g_m + help[az_idx].first.size();
        } while (vis[node_idx] == true || help[az_idx].second == area_upper);
        stripe_nodes.push_back(node_idx);
        m_Node_info[node_idx].stripes.insert(stripe_id);
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
        m_Node_info[node_idx].stripes.insert(stripe_id);
        vis[node_idx] = true;
        if (help[az_idx].first.count(i) == 0) {
          help[az_idx].first.insert(i);
        }
        help[az_idx].second++;
      }
      if (tail_group > 0) {
        do {
          node_idx = dis(gen);
          az_idx = m_Node_info[node_idx].AZ_id;
          area_upper = g_m + help[az_idx].first.size();
        } while (vis[node_idx] == true || help[az_idx].second == area_upper);
        stripe_nodes.push_back(node_idx);
        m_Node_info[node_idx].stripes.insert(stripe_id);
        vis[node_idx] = true;
        if (help[az_idx].first.count(l) == 0) {
          help[az_idx].first.insert(l);
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
      m_Node_info[node_idx].stripes.insert(stripe_id);
      vis[node_idx] = true;
      help[az_idx].second++;
    } else if (placement_type == Best_Placement) {
      int start_idx = 0;
      int sita = g_m / b;
      int num_nodes = tail_group > 0 ? (k + g_m + l + 1 + 1) : (k + g_m + l + 1);
      stripe_nodes.resize(num_nodes);
      if (sita >= 1) {
        int left_data_shard = k;
        while (left_data_shard > 0) {
          if (left_data_shard >= sita * b) {
            cur_az = cur_az % m_AZ_info.size();
            AZitem &az = m_AZ_info[cur_az++];
            for (int i = 0; i < sita * b; i++) {
              cur_node = cur_node % az.nodes.size();
              stripe_nodes[start_idx + i] = az.nodes[cur_node++];
              m_Node_info[az.nodes[cur_node - 1]].stripes.insert(stripe_id);
            }
            for (int i = 0; i < sita; i++) {
              cur_node = cur_node % az.nodes.size();
              stripe_nodes[k + g_m + start_idx / b + i] = az.nodes[cur_node++];
              m_Node_info[az.nodes[cur_node - 1]].stripes.insert(stripe_id);
            }
            start_idx += (sita * b);
            left_data_shard -= (sita * b);
          } else {
            int left_group = left_data_shard / b;
            cur_az = cur_az % m_AZ_info.size();
            AZitem &az = m_AZ_info[cur_az++];
            for (int i = 0; i < left_group * b; i++) {
              cur_node = cur_node % az.nodes.size();
              stripe_nodes[start_idx + i] = az.nodes[cur_node++];
              m_Node_info[az.nodes[cur_node - 1]].stripes.insert(stripe_id);
            }
            for (int i = 0; i < left_group; i++) {
              cur_node = cur_node % az.nodes.size();
              stripe_nodes[k + g_m + start_idx / b + i] = az.nodes[cur_node++];
              m_Node_info[az.nodes[cur_node - 1]].stripes.insert(stripe_id);
            }
            start_idx += (left_group * b);
            left_data_shard -= (left_group * b);
            if (left_data_shard > 0) {
              for (int i = 0; i < left_data_shard; i++) {
                cur_node = cur_node % az.nodes.size();
                stripe_nodes[start_idx + i] = az.nodes[cur_node++];
                m_Node_info[az.nodes[cur_node - 1]].stripes.insert(stripe_id);
              }
              cur_node = cur_node % az.nodes.size();
              stripe_nodes[k + g_m + start_idx / b] = az.nodes[cur_node++];
              m_Node_info[az.nodes[cur_node - 1]].stripes.insert(stripe_id);
              left_data_shard -= left_data_shard;
            }
          }
        }
        cur_az = cur_az % m_AZ_info.size();
        AZitem &az = m_AZ_info[cur_az++];
        for (int i = 0; i < g_m; i++) {
          cur_node = cur_node % az.nodes.size();
          stripe_nodes[k + i] = az.nodes[cur_node++];
          m_Node_info[az.nodes[cur_node - 1]].stripes.insert(stripe_id);
        }
        cur_node = cur_node % az.nodes.size();
        stripe_nodes[stripe_nodes.size() - 1] = az.nodes[cur_node++];
        m_Node_info[az.nodes[cur_node - 1]].stripes.insert(stripe_id);
      } else {
        int idx = 0;
        for (int i = 0; i <= l; i++) {
          int left_data_shard_in_group = b;
          if (i == l) {
            if (tail_group <= 0) {
              continue;
            }
          }
          if (i == l) {
            left_data_shard_in_group = tail_group;
          }
          while (left_data_shard_in_group >= g_m + 1) {
            cur_az = cur_az % m_AZ_info.size();
            AZitem &az = m_AZ_info[cur_az++];
            for (int j = 0; j < g_m + 1; j++) {
              cur_node = cur_node % az.nodes.size();
              stripe_nodes[idx++] = az.nodes[cur_node++];
              m_Node_info[az.nodes[cur_node - 1]].stripes.insert(stripe_id);
            }
            left_data_shard_in_group -= (g_m + 1);
          }
          if (left_data_shard_in_group == 0) {
            cur_az = cur_az % m_AZ_info.size();
            AZitem &az = m_AZ_info[cur_az++];
            cur_node = cur_node % az.nodes.size();
            stripe_nodes[k + g_m + i] = az.nodes[cur_node++];
            m_Node_info[az.nodes[cur_node - 1]].stripes.insert(stripe_id);
            continue;
          }
          cur_az = cur_az % m_AZ_info.size();
          AZitem &az = m_AZ_info[cur_az++];
          for (int i = 0; i < left_data_shard_in_group; i++) {
            cur_node = cur_node % az.nodes.size();
            stripe_nodes[idx++] = az.nodes[cur_node++];
            m_Node_info[az.nodes[cur_node - 1]].stripes.insert(stripe_id);
          }
          cur_node = cur_node % az.nodes.size();
          stripe_nodes[k + g_m + i] = az.nodes[cur_node++];
          m_Node_info[az.nodes[cur_node - 1]].stripes.insert(stripe_id);
        }
        cur_az = cur_az % m_AZ_info.size();
        AZitem &az = m_AZ_info[cur_az++];
        for (int i = 0; i < g_m; i++) {
          cur_node = cur_node % az.nodes.size();
          stripe_nodes[k + i] = az.nodes[cur_node++];
          m_Node_info[az.nodes[cur_node - 1]].stripes.insert(stripe_id);
        }
        cur_node = cur_node % az.nodes.size();
        stripe_nodes[stripe_nodes.size() - 1] = az.nodes[cur_node++];
        m_Node_info[az.nodes[cur_node - 1]].stripes.insert(stripe_id);
      }
    }
  } else if (encode_type == OPPO_LRC) {
    // OPPO_LRC1个group放1个az
    int real_l = tail_group > 0 ? (l + 1) : (l);
    std::vector<int> az_ids;
    for (int i = 0; i < real_l; i++) {
      cur_az = cur_az % m_AZ_info.size();
      AZitem &az = m_AZ_info[cur_az++];
      az_ids.push_back(az.AZ_id);
    }
    for (int i = 0; i < l; i++) {
      AZitem &az = m_AZ_info[az_ids[i]];
      for (int j = 0; j < b; j++) {
        cur_node = cur_node % az.nodes.size();
        stripe_nodes.push_back(az.nodes[cur_node++]);
        m_Node_info[az.nodes[cur_node - 1]].stripes.insert(stripe_id);
      }
    }
    if (tail_group > 0) {
      AZitem &az = m_AZ_info[az_ids[l]];
      for (int i = 0; i < tail_group; i++) {
        cur_node = cur_node % az.nodes.size();
        stripe_nodes.push_back(az.nodes[cur_node++]);
        m_Node_info[az.nodes[cur_node - 1]].stripes.insert(stripe_id);
      }
    }
    std::vector<int> g_num_per_az(az_ids.size(), 0);
    int idx = 0;
    for (int i = 0; i < g_m; i++) {
      idx = idx % g_num_per_az.size();
      g_num_per_az[idx++]++;
    }
    for (int i = 0; i < az_ids.size(); i++) {
      AZitem &az = m_AZ_info[az_ids[i]];
      for (int j = 0; j < g_num_per_az[i]; j++) {
        cur_node = cur_node % az.nodes.size();
        stripe_nodes.push_back(az.nodes[cur_node++]);
        m_Node_info[az.nodes[cur_node - 1]].stripes.insert(stripe_id);
      }
    }
    for (int i = 0; i < az_ids.size(); i++) {
      AZitem &az = m_AZ_info[az_ids[i]];
      cur_node = cur_node % az.nodes.size();
      stripe_nodes.push_back(az.nodes[cur_node++]);
      m_Node_info[az.nodes[cur_node - 1]].stripes.insert(stripe_id);
    }
  }
  // for (int i = 0; i < l; i++) {
  //   for (int j = 0; j < b; j++) {
  //     std::cout << stripe_nodes[i * b + j] << " ";
  //   }
  //   std::cout << std::endl;
  // }
  // if (tail_group > 0) {
  //   for (int i = 0; i < tail_group; i++) {
  //     std::cout << stripe_nodes[l * b + i] << " ";
  //   }
  //   std::cout << std::endl;
  // }
  // for (int i = 0; i < g_m; i++) {
  //   std::cout << stripe_nodes[k + i] << " ";
  // }
  // std::cout << std::endl;
  // int real_l_include_gl = tail_group > 0 ? (l + 1 + 1) : (l + 1);
  // for (int i = 0; i < real_l_include_gl; i++) {
  //   std::cout << stripe_nodes[k + g_m + i] << " ";
  // }
  // std::cout << std::endl;
  // std::cout << "******************************" << std::endl;
  return;
}

} // namespace OppoProject