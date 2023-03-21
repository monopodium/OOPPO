#include "coordinator.h"
#include "tinyxml2.h"
#include <random>
#include <thread>
#include "azure_lrc.h"

namespace OppoProject
{

  template <typename T>
  inline T ceil(T const &A, T const &B)
  {
    return T((A + B - 1) / B);
  };

  grpc::Status CoordinatorImpl::setParameter(
      ::grpc::ServerContext *context,
      const coordinator_proto::Parameter *parameter,
      coordinator_proto::RepIfSetParaSucess *setParameterReply)
  {
    ECSchema system_metadata(parameter->partial_decoding(),
                             (OppoProject::EncodeType)parameter->encodetype(),
                             (OppoProject::PlacementType)parameter->placementtype(),
                             parameter->k_datablock(),
                             parameter->real_l_localgroup(),
                             parameter->g_m_globalparityblock(),
                             parameter->b_datapergoup(),
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
      coordinator_proto::ReplyFromCoordinator *helloReplyFromCoordinator)
  {
    std::string prefix("Hello ");
    helloReplyFromCoordinator->set_message(prefix +
                                           helloRequestToCoordinator->name());
    std::cout << prefix + helloRequestToCoordinator->name() << std::endl;
    return grpc::Status::OK;
  }

  grpc::Status CoordinatorImpl::uploadOriginKeyValue(
      ::grpc::ServerContext *context,
      const coordinator_proto::RequestProxyIPPort *keyValueSize,
      coordinator_proto::ReplyProxyIPPort *proxyIPPort)
  {

    std::string key = keyValueSize->key();
    m_mutex.lock();
    m_object_table_big_small_commit.erase(key);
    m_mutex.unlock();
    int valuesizebytes = keyValueSize->valuesizebytes();

    ObjectItemBigSmall new_object;

    /*编码参数*/
    int k = m_encode_parameter.k_datablock;
    int m = m_encode_parameter.g_m_globalparityblock;
    int real_l = m_encode_parameter.real_l_localgroup;
    int b = m_encode_parameter.b_datapergoup;
    new_object.object_size = valuesizebytes;
    /*文件分为三类：
    超大文件：value_size > blob_size_upper
    大文件：blob_size_upper >= value_size > small_file_upper
    小文件：small_file_upper >= value_size
    */

    /**meta data update*/
    if (valuesizebytes > m_encode_parameter.small_file_upper)
    {
      new_object.big_object = true;
      if (valuesizebytes > m_encode_parameter.blob_size_upper)
      {
        proxy_proto::ObjectAndPlacement object_placement;
        object_placement.set_encode_type((int)m_encode_parameter.encodetype);
        object_placement.set_bigobject(true);
        object_placement.set_key(key);
        object_placement.set_valuesizebyte(valuesizebytes);
        object_placement.set_k(k);
        object_placement.set_m(m);
        object_placement.set_real_l(real_l);
        object_placement.set_b(b);
        int shard_size = ceil(m_encode_parameter.blob_size_upper, k);
        shard_size = 16 * ceil(shard_size, 16);
        object_placement.set_shard_size(shard_size);

        int num_of_stripes = valuesizebytes / (k * shard_size);
        for (int i = 0; i < num_of_stripes; i++)
        {
          valuesizebytes -= k * shard_size;
          StripeItem temp;
          temp.Stripe_id = m_next_stripe_id++;
          m_Stripe_info[temp.Stripe_id] = temp;
          StripeItem &stripe = m_Stripe_info[temp.Stripe_id];
          stripe.shard_size = shard_size;
          stripe.k = k;
          stripe.real_l = real_l;
          stripe.g_m = m;
          stripe.b = b;
          stripe.placementtype = m_encode_parameter.placementtype;
          stripe.encodetype = m_encode_parameter.encodetype;
          // for (int i = 0; i < k + m; i++) {
          //   // 其实应该根据placement_plan来添加node_id
          //   stripe.nodes.push_back(i);
          // }
          generate_placement(m_Stripe_info[stripe.Stripe_id].nodes, stripe.Stripe_id);
          new_object.stripes.push_back(stripe.Stripe_id);

          object_placement.add_stripe_ids(stripe.Stripe_id);
          for (int i = 0; i < int(stripe.nodes.size()); i++)
          {
            Nodeitem &node = m_Node_info[stripe.nodes[i]];
            object_placement.add_datanodeip(node.Node_ip.c_str());
            object_placement.add_datanodeport(node.Node_port);
          }
        }
        if (valuesizebytes > 0)
        {
          int shard_size = ceil(valuesizebytes, k);
          shard_size = 16 * ceil(shard_size, 16);
          StripeItem temp;
          temp.Stripe_id = m_next_stripe_id++;
          m_Stripe_info[temp.Stripe_id] = temp;
          StripeItem &stripe = m_Stripe_info[temp.Stripe_id];
          stripe.shard_size = shard_size;
          stripe.k = k;
          stripe.real_l = real_l;
          stripe.g_m = m;
          stripe.b = b;
          stripe.placementtype = m_encode_parameter.placementtype;
          stripe.encodetype = m_encode_parameter.encodetype;
          // for (int i = 0; i < k + m; i++) {
          //   // 其实应该根据placement_plan来添加node_id
          //   stripe.nodes.push_back(i);
          // }
          generate_placement(m_Stripe_info[stripe.Stripe_id].nodes, stripe.Stripe_id);
          new_object.stripes.push_back(stripe.Stripe_id);

          object_placement.add_stripe_ids(stripe.Stripe_id);
          object_placement.set_tail_shard_size(shard_size);
          for (int i = 0; i < int(stripe.nodes.size()); i++)
          {
            Nodeitem &node = m_Node_info[stripe.nodes[i]];
            object_placement.add_datanodeip(node.Node_ip.c_str());
            object_placement.add_datanodeport(node.Node_port);
          }
        }
        else
        {
          object_placement.set_tail_shard_size(-1);
        }
        grpc::ClientContext handle_ctx;
        proxy_proto::SetReply set_reply;
        grpc::Status status;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<unsigned int> dis(0, m_AZ_info.size() - 1);
        int az_id = dis(gen);
        std::string selected_proxy_ip = m_AZ_info[az_id].proxy_ip;
        int selected_proxy_port = m_AZ_info[az_id].proxy_port;
        std::string choose_proxy = selected_proxy_ip + ":" + std::to_string(selected_proxy_port);
        status = m_proxy_ptrs[choose_proxy]->EncodeAndSetObject(&handle_ctx, object_placement, &set_reply);
        proxyIPPort->set_proxyip(selected_proxy_ip);
        proxyIPPort->set_proxyport(selected_proxy_port + 1);
      }
      else
      {
        int shard_size = ceil(valuesizebytes, k);
        shard_size = 16 * ceil(shard_size, 16);
        StripeItem temp;
        temp.Stripe_id = m_next_stripe_id++;
        m_Stripe_info[temp.Stripe_id] = temp;
        StripeItem &stripe = m_Stripe_info[temp.Stripe_id];
        stripe.shard_size = shard_size;
        stripe.k = k;
        stripe.real_l = real_l;
        stripe.g_m = m;
        stripe.b = b;
        stripe.placementtype = m_encode_parameter.placementtype;
        stripe.encodetype = m_encode_parameter.encodetype;
        // for (int i = 0; i < k + m; i++) {
        //   // 其实应该根据placement_plan来添加node_id
        //   stripe.nodes.push_back(i);
        // }
        generate_placement(m_Stripe_info[stripe.Stripe_id].nodes, stripe.Stripe_id);
        new_object.stripes.push_back(stripe.Stripe_id);

        grpc::ClientContext handle_ctx;
        proxy_proto::SetReply set_reply;
        grpc::Status status;
        proxy_proto::ObjectAndPlacement object_placement;

        object_placement.set_encode_type((int)stripe.encodetype);
        object_placement.add_stripe_ids(stripe.Stripe_id);
        object_placement.set_bigobject(true);
        object_placement.set_key(key);
        object_placement.set_valuesizebyte(valuesizebytes);
        object_placement.set_k(k);
        object_placement.set_m(m);
        object_placement.set_real_l(real_l);
        object_placement.set_b(b);
        object_placement.set_shard_size(shard_size);
        object_placement.set_tail_shard_size(-1);
        for (int i = 0; i < int(stripe.nodes.size()); i++)
        {
          Nodeitem &node = m_Node_info[stripe.nodes[i]];
          object_placement.add_datanodeip(node.Node_ip.c_str());
          object_placement.add_datanodeport(node.Node_port);
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<unsigned int> dis(0, m_AZ_info.size() - 1);
        int az_id = dis(gen);
        std::string selected_proxy_ip = m_AZ_info[az_id].proxy_ip;
        int selected_proxy_port = m_AZ_info[az_id].proxy_port;
        std::string choose_proxy = selected_proxy_ip + ":" + std::to_string(selected_proxy_port);
        status = m_proxy_ptrs[choose_proxy]->EncodeAndSetObject(&handle_ctx, object_placement, &set_reply);
        proxyIPPort->set_proxyip(selected_proxy_ip);
        proxyIPPort->set_proxyport(selected_proxy_port + 1);

        if (status.ok())
        {
        }
        else
        {
          std::cout << "datanodes can not serve client download request!"
                    << std::endl;
          return grpc::Status::CANCELLED;
        }
      }
    }
    else
    {
      /*Small Object Write*/
      // std::cout << "enter uploadOriginKeyValue smallwrite branch" <<std::endl;
      new_object.big_object = false;
      int shard_size = m_encode_parameter.blob_size_upper;

      /*init private member buf_rest,cur_stripe and az_id_for_cur_stripe*/
      static int init_flag = true;
      static std::vector<int> temp_buf_rest(k, m_encode_parameter.blob_size_upper);
      static StripeItem temp = {
          .Stripe_id = m_next_stripe_id++,
          .shard_size = shard_size,
          .k = k,
          .real_l = real_l,
          .g_m = m,
          .b = b,
          .encodetype = m_encode_parameter.encodetype,
          .placementtype = m_encode_parameter.placementtype,
      };
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<unsigned int> dis(0, m_AZ_info.size() - 1);
      static int az_id = dis(gen);
      std::string selected_proxy_ip = m_AZ_info[az_id_for_cur_stripe].proxy_ip;
      int selected_proxy_port = m_AZ_info[az_id_for_cur_stripe].proxy_port;
      //selected_proxy_port = 50005;
      // std::string  selected_proxy_ip = "0.0.0.0";
      // std::string choose_proxy = selected_proxy_ip + ":" + std::to_string(selected_proxy_port);
      if (init_flag)
      {
        buf_rest = temp_buf_rest;
        cur_stripe = temp;
        m_Stripe_info[cur_stripe.Stripe_id] = cur_stripe;
        az_id_for_cur_stripe = az_id;
        init_flag = false;
        cur_smallobj_proxy_ip = selected_proxy_ip;
        cur_smallobj_proxy_port = selected_proxy_port;
        cur_smallobj_proxy_ip_port = cur_smallobj_proxy_ip + ":" + std::to_string(cur_smallobj_proxy_port);
        std::cout << "init_proxy is : " << cur_smallobj_proxy_ip_port << std::endl;
      }

      /*object metadata record*/
      grpc::ClientContext handle_ctx;
      proxy_proto::SetReply set_reply;
      grpc::Status status;
      proxy_proto::ObjectAndPlacement object_placement;

      object_placement.set_encode_type((int)cur_stripe.encodetype);
      object_placement.set_bigobject(false);
      object_placement.set_key(key);
      object_placement.set_valuesizebyte(valuesizebytes);
      object_placement.set_k(k);
      object_placement.set_m(m);
      object_placement.set_real_l(real_l);
      object_placement.set_b(b);
      object_placement.set_shard_size(shard_size);
      object_placement.set_tail_shard_size(-1);
      object_placement.add_stripe_ids(cur_stripe.Stripe_id);

      /*check buffer(simple-version)*/
      int buf_idx = -1;
      for (int i = 0; i < k; i++)
      {
        if (buf_rest[i] >= valuesizebytes)
          if (buf_idx == -1)
            buf_idx = i;
          else if (buf_rest[i] >= buf_rest[buf_idx])
            buf_idx = i;
      }

      if (buf_idx == -1)
      {
        /*encode old buffer*/
        object_placement.set_writebufferindex(buf_idx);
        generate_placement(m_Stripe_info[cur_stripe.Stripe_id].nodes, cur_stripe.Stripe_id);
        for (int i = 0; i < int(m_Stripe_info[cur_stripe.Stripe_id].nodes.size()); i++)
        {
          Nodeitem &node = m_Node_info[m_Stripe_info[cur_stripe.Stripe_id].nodes[i]];
          object_placement.add_datanodeip(node.Node_ip.c_str());
          object_placement.add_datanodeport(node.Node_port);
        }
        cur_smallobj_proxy_ip_port = cur_smallobj_proxy_ip + ":" + std::to_string(cur_smallobj_proxy_port);
        status = m_proxy_ptrs[cur_smallobj_proxy_ip_port]->WriteBufferAndEncode(
            &handle_ctx, object_placement, &set_reply);
        proxyIPPort->set_proxyip(cur_smallobj_proxy_ip);
        proxyIPPort->set_proxyport(cur_smallobj_proxy_port + 1);
        if (status.ok())
        {
          std::cout << "encode buffer success!"
                    << std::endl;
        }
        else
        {
          std::cout << "encode buffer fail!"
                    << std::endl;
          return grpc::Status::CANCELLED;
        }
        // TODO
        /*generate new stripe*/
        cur_stripe.Stripe_id = m_next_stripe_id++;
        cur_stripe.shard_size = shard_size;
        cur_stripe.k = k;
        cur_stripe.real_l = real_l;
        cur_stripe.g_m = m;
        cur_stripe.b = b;
        cur_stripe.placementtype = m_encode_parameter.placementtype;
        cur_stripe.encodetype = m_encode_parameter.encodetype;
        m_Stripe_info[cur_stripe.Stripe_id] = cur_stripe;
        /*generate new proxy_ip*/
        az_id_for_cur_stripe = dis(gen);
        cur_smallobj_proxy_ip = m_AZ_info[az_id_for_cur_stripe].proxy_ip;
        cur_smallobj_proxy_port = m_AZ_info[az_id_for_cur_stripe].proxy_port;
        // selected_proxy_port = 50005;
        cur_smallobj_proxy_ip_port = cur_smallobj_proxy_ip + ":" + std::to_string(cur_smallobj_proxy_port);
        std::cout << "new_proxy is : " << cur_smallobj_proxy_ip_port << std::endl;
        /* (1) Reinit the buf_rest;
           (2) the new object will be writed into the buffer[0]*/
        for (int i = 0; i < k; i++)
        {
          buf_rest[i] = shard_size;
        }
        buf_idx = 0;
        key_in_buffer.clear();
      }
      /*write buffer*/
      /*std::cout << "enter uploadOriginKeyValue smallwrite write buffer branch" <<std::endl; */
      grpc::ClientContext new_handle_ctx;
      object_placement.set_writebufferindex(buf_idx);
      object_placement.add_stripe_ids(cur_stripe.Stripe_id);
      status = m_proxy_ptrs[cur_smallobj_proxy_ip_port]->WriteBufferAndEncode(
          &new_handle_ctx, object_placement, &set_reply);
      proxyIPPort->set_proxyip(cur_smallobj_proxy_ip);
      proxyIPPort->set_proxyport(cur_smallobj_proxy_port + 1);
      if (status.ok())
      {
        //std::cout << "write buffer success!" << std::endl;
        key_in_buffer.insert(key);
      }
      else
      {
        std::cout << "write buffer failed!"
                  << std::endl;
        std::cout << status.error_message() << std::endl;
        return grpc::Status::CANCELLED;
      }
      new_object.stripes.push_back(cur_stripe.Stripe_id);
      new_object.shard_idx = buf_idx;
      new_object.offset = shard_size - buf_rest[buf_idx];
      new_object.object_size = valuesizebytes;
      buf_rest[buf_idx] -= valuesizebytes;
    }
    std::unique_lock<std::mutex> lck(m_mutex);
    m_object_table_big_small_updating[key] = new_object;

    /*inform proxy*/

    return grpc::Status::OK;
  }

  grpc::Status
  CoordinatorImpl::getValue(::grpc::ServerContext *context,
                            const coordinator_proto::KeyAndClientIP *keyClient,
                            coordinator_proto::RepIfGetSucess *getReplyClient)
  {
    try
    {
      std::string key = keyClient->key();
      std::string client_ip = keyClient->clientip();
      int client_port = keyClient->clientport();
      m_mutex.lock();
      ObjectItemBigSmall object_infro = m_object_table_big_small_commit.at(key);
      m_mutex.unlock();
      int k = m_Stripe_info[object_infro.stripes[0]].k;
      int m = m_Stripe_info[object_infro.stripes[0]].g_m;
      int real_l = m_Stripe_info[object_infro.stripes[0]].real_l;
      int b = m_Stripe_info[object_infro.stripes[0]].b;

      grpc::ClientContext decode_and_get;
      proxy_proto::ObjectAndPlacement object_placement;
      grpc::Status status;
      proxy_proto::GetReply get_reply;
      getReplyClient->set_valuesizebytes(object_infro.object_size);
      if (object_infro.big_object)
      { /*大文件读*/
        object_placement.set_bigobject(true);
        object_placement.set_key(key);
        object_placement.set_valuesizebyte(object_infro.object_size);
        object_placement.set_k(k);
        object_placement.set_m(m);
        object_placement.set_real_l(real_l);
        object_placement.set_b(b);
        object_placement.set_tail_shard_size(-1);
        if (object_infro.object_size > m_encode_parameter.blob_size_upper)
        {
          int shard_size = ceil(m_encode_parameter.blob_size_upper, k);
          shard_size = 16 * ceil(shard_size, 16);
          object_placement.set_shard_size(shard_size);
          if (object_infro.object_size % (k * shard_size) != 0)
          {
            int tail_stripe_size = object_infro.object_size % (k * shard_size);
            int tail_shard_size = ceil(tail_stripe_size, k);
            tail_shard_size = 16 * ceil(tail_shard_size, 16);
            object_placement.set_tail_shard_size(tail_shard_size);
          }
        }
        else
        {
          int shard_size = ceil(object_infro.object_size, k);
          shard_size = 16 * ceil(shard_size, 16);
          object_placement.set_shard_size(shard_size);
        }

        for (int i = 0; i < int(object_infro.stripes.size()); i++)
        {
          StripeItem &stripe = m_Stripe_info[object_infro.stripes[i]];
          object_placement.set_encode_type((int)stripe.encodetype);
          object_placement.add_stripe_ids(stripe.Stripe_id);
          for (int j = 0; j < int(stripe.nodes.size()); j++)
          {
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
      }
      else
      { //小文件读
        object_placement.set_bigobject(false);
        object_placement.set_key(key);
        object_placement.set_valuesizebyte(object_infro.object_size);
        object_placement.set_k(k);
        object_placement.set_m(m);
        object_placement.set_real_l(real_l);
        object_placement.set_b(b);
        object_placement.set_tail_shard_size(-1);
        int shard_size = ceil(m_encode_parameter.blob_size_upper, k);
        shard_size = 16 * ceil(shard_size, 16);
        object_placement.set_shard_size(shard_size);
        object_placement.set_offset(object_infro.offset);
        object_placement.set_shard_idx(object_infro.shard_idx);
        object_placement.set_obj_size(object_infro.object_size);
        object_placement.set_clientip(client_ip);
        object_placement.set_clientport(client_port); 
        // std::cout << "metadata: key is " << object_placement.key() << std::endl;
        // std::cout << "metadata: size is " << object_placement.obj_size() << std::endl;
        // std::cout << "metadata: offset is " << object_placement.offset() << std::endl;
        // std::cout << "metadata: stripeID is " << stripe.Stripe_id << std::endl;
        // std::cout << "metadata: chunckIdx is " << object_placement.shard_idx() << std::endl;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<unsigned int> dis(0, m_AZ_info.size() - 1);
        std::string choose_proxy = m_AZ_info[dis(gen)].proxy_ip + ":" + std::to_string(m_AZ_info[dis(gen)].proxy_port);
        // choose_proxy =  m_AZ_info[0].proxy_ip + ":" + std::to_string(m_AZ_info[0].proxy_port);// 使用50005端口读小文件
        if(key_in_buffer.count(object_placement.key()) == 0){
          StripeItem &stripe = m_Stripe_info[object_infro.stripes[0]];
          object_placement.set_encode_type((int)stripe.encodetype);
          object_placement.add_stripe_ids(stripe.Stripe_id);
          Nodeitem &node = m_Node_info[stripe.nodes[object_infro.shard_idx]];
          object_placement.add_datanodeip(node.Node_ip.c_str());
          object_placement.add_datanodeport(node.Node_port);
          status = m_proxy_ptrs[choose_proxy]->decodeAndGetObject(&decode_and_get, object_placement, &get_reply);
          // std::cout << "small file read from memcached servers finish!" << std::endl;
        } else {
          status = m_proxy_ptrs[cur_smallobj_proxy_ip_port]->getObjectFromBuffer(&decode_and_get, object_placement, &get_reply);
          // std::cout << "small file read from buffer finish!" << std::endl;
        }
      }
    }
    catch (std::exception &e)
    {
      std::cout << "getValue exception" << std::endl;
      std::cout << e.what() << std::endl;
    }
    return grpc::Status::OK;
  }
  grpc::Status CoordinatorImpl::checkalive(
      grpc::ServerContext *context,
      const coordinator_proto::RequestToCoordinator *helloRequestToCoordinator,
      coordinator_proto::ReplyFromCoordinator *helloReplyFromCoordinator)
  {

    std::cout << "checkalive" << helloRequestToCoordinator->name() << std::endl;
    return grpc::Status::OK;
  }
  grpc::Status CoordinatorImpl::reportCommitAbort(
      grpc::ServerContext *context,
      const coordinator_proto::CommitAbortKey *commit_abortkey,
      coordinator_proto::ReplyFromCoordinator *helloReplyFromCoordinator)
  {
    std::string key = commit_abortkey->key();
    std::unique_lock<std::mutex> lck(m_mutex);
    try
    {
      if (commit_abortkey->ifcommitmetadata())
      {
        m_object_table_big_small_commit[key] = m_object_table_big_small_updating[key];
        cv.notify_all();

        m_object_table_big_small_updating.erase(key);
      }
      else
      {
        m_object_table_big_small_updating.erase(key);
      }
    }
    catch (std::exception &e)
    {
      std::cout << "reportCommitAbort exception" << std::endl;
      std::cout << e.what() << std::endl;
    }
    return grpc::Status::OK;
  }

  grpc::Status
  CoordinatorImpl::checkCommitAbort(grpc::ServerContext *context,
                                    const coordinator_proto::AskIfSetSucess *key,
                                    coordinator_proto::RepIfSetSucess *reply)
  {
    std::unique_lock<std::mutex> lck(m_mutex);
    while (m_object_table_big_small_commit.find(key->key()) ==
           m_object_table_big_small_commit.end())
    {
      cv.wait(lck);
    }
    reply->set_ifcommit(true);
    /*待补充*/
    return grpc::Status::OK;
  }

  grpc::Status CoordinatorImpl::requestRepair(::grpc::ServerContext *context,
                                              const coordinator_proto::FailNodes *failed_node_list,
                                              coordinator_proto::RepIfRepairSucess *reply)
  {
    std::vector<int> failed_node_ids;
    for (int i = 0; i < failed_node_list->node_list_size(); i++)
    {
      int node_id = failed_node_list->node_list(i);
      failed_node_ids.push_back(node_id);
    }

    std::unordered_set<int> failed_stripe_ids;
    for (auto &node_id : failed_node_ids)
    {
      for (auto &stripe_id : m_Node_info[node_id].stripes)
      {
        failed_stripe_ids.insert(stripe_id);
      }
    }
    for (auto &stripe_id : failed_stripe_ids)
    {
      StripeItem &stripe_info = m_Stripe_info[stripe_id];
      std::vector<int> failed_shard_idxs;
      for (size_t i = 0; i < m_Stripe_info[stripe_id].nodes.size(); i++)
      {
        auto lookup = std::find(failed_node_ids.begin(), failed_node_ids.end(), m_Stripe_info[stripe_id].nodes[i]);
        if (lookup != failed_node_ids.end())
        {
          failed_shard_idxs.push_back(i);
        }
      }

      // 首先判断能不能解码：
      int survive_block = 0;
      for (int i = 0; i < stripe_info.k + stripe_info.g_m + stripe_info.real_l; i++)
      {
        if (std::find(failed_shard_idxs.begin(), failed_shard_idxs.end(), i) == failed_shard_idxs.end())
        {
          survive_block++;
        }
      }
      if (survive_block == stripe_info.k)
      {
        std::vector<int> matrix((stripe_info.g_m + stripe_info.real_l) * stripe_info.k, 0);
        lrc_make_matrix(stripe_info.k, stripe_info.g_m, stripe_info.real_l, matrix.data());
        if (check_decodable_azure_lrc(stripe_info.k, stripe_info.g_m, stripe_info.real_l, failed_shard_idxs, matrix) != 1)
        {
          std::cout << "Not decodable!!!" << std::endl;
          continue;
        }
      }

      if (failed_shard_idxs.size() == 0)
      {
        continue;
      }
      std::unordered_map<int, std::vector<int>> failed_shard_idxs_in_each_az;
      for (int i = 0; i < int(failed_shard_idxs.size()); i++)
      {
        Nodeitem &node_info = m_Node_info[stripe_info.nodes[failed_shard_idxs[i]]];
        failed_shard_idxs_in_each_az[node_info.AZ_id].push_back(failed_shard_idxs[i]);
      }
      std::vector<int> real_failed_shard_idxs;
      // 对于OPPO_LRC，先检查每个组中放置的可以修复的，但是这样写现在是默认一个az里面只有一个组了，所以可能需要修改
      for (auto &p : failed_shard_idxs_in_each_az)
      {
        if (p.second.size() == 1 && stripe_info.encodetype == OPPO_LRC)
        {
          do_repair(stripe_id, p.second);
        }
        else
        {
          for (auto &q : p.second)
          {
            real_failed_shard_idxs.push_back(q);
          }
        }
      }
      if (real_failed_shard_idxs.size() > 0)
      {
        do_repair(stripe_id, real_failed_shard_idxs);
      }
      int block_bound_need_single_repair = (stripe_info.encodetype == Azure_LRC_1) ? (stripe_info.k + stripe_info.g_m + stripe_info.real_l) : (stripe_info.k + stripe_info.g_m);
      for (int i = 0; i < int(real_failed_shard_idxs.size()); i++)
      {
        if (real_failed_shard_idxs[i] >= block_bound_need_single_repair)
        {
          do_repair(stripe_id, {real_failed_shard_idxs[i]});
        }
      }
    }
    reply->set_ifrepair(true);
    return grpc::Status::OK;
  }

  bool num_survive_nodes(std::pair<int, std::vector<std::pair<int, int>>> &a, std::pair<int, std::vector<std::pair<int, int>>> &b)
  {
    return a.second.size() > b.second.size();
  }

  bool cmp_num_live_shards(std::pair<int, std::vector<int>> &a, std::pair<int, std::vector<int>> &b)
  {
    return a.second > b.second;
  }

  void CoordinatorImpl::generate_repair_plan(int stripe_id, bool one_shard, std::vector<int> &failed_shard_idxs,
                                             std::vector<std::vector<std::pair<std::pair<std::string, int>, int>>> &shards_to_read,
                                             std::vector<int> &repair_span_az,
                                             std::vector<std::pair<int, int>> &new_locations_with_shard_idx,
                                             std::unordered_map<int, bool> &merge)
  {
    StripeItem &stripe_info = m_Stripe_info[stripe_id];
    int k = stripe_info.k;
    int real_l = stripe_info.real_l;
    int g = stripe_info.g_m;
    int b = stripe_info.b;
    if (one_shard)
    {
      int failed_shard_idx = failed_shard_idxs[0];
      Nodeitem &failed_node_info = m_Node_info[stripe_info.nodes[failed_shard_idx]];
      int main_az_id = failed_node_info.AZ_id;
      repair_span_az.push_back(main_az_id);
      for (int i = 0; i < int(m_AZ_info[main_az_id].nodes.size()); i++)
      {
        int node_id = m_AZ_info[main_az_id].nodes[i];
        auto lookup = std::find(stripe_info.nodes.begin(), stripe_info.nodes.end(), node_id);
        if (lookup == stripe_info.nodes.end())
        {
          new_locations_with_shard_idx.push_back({node_id, failed_shard_idx});
          break;
        }
      }
      if (stripe_info.encodetype == RS)
      {
        std::vector<int> shard_idx_for_repair;
        if (failed_shard_idx >= 0 && failed_shard_idx <= (k - 1))
        {
          for (int i = 0; i < k; i++)
          {
            if (i != failed_shard_idx)
            {
              shard_idx_for_repair.push_back(i);
            }
          }
          shard_idx_for_repair.push_back(k);
        }
        else if (failed_shard_idx >= k && failed_shard_idx <= (k + g - 1))
        {
          for (int i = 0; i < k; i++)
          {
            shard_idx_for_repair.push_back(i);
          }
        }
        std::unordered_map<int, std::vector<int>> azs_and_shards_idx;
        std::vector<std::pair<std::pair<std::string, int>, int>> shards_to_read_for_main_az;
        for (int i = 0; i < int(shard_idx_for_repair.size()); i++)
        {
          int shard_idx = shard_idx_for_repair[i];
          Nodeitem &node_info = m_Node_info[stripe_info.nodes[shard_idx]];
          if (node_info.AZ_id == main_az_id)
          {
            shards_to_read_for_main_az.push_back({{node_info.Node_ip, node_info.Node_port}, shard_idx});
          }
          else
          {
            azs_and_shards_idx[node_info.AZ_id].push_back(shard_idx);
          }
        }
        shards_to_read.push_back(shards_to_read_for_main_az);
        for (auto &az_and_shards_idx : azs_and_shards_idx)
        {
          int az_id = az_and_shards_idx.first;
          repair_span_az.push_back(az_id);
          std::vector<std::pair<std::pair<std::string, int>, int>> shards_to_read_for_help_az;
          for (auto &shard_idx : az_and_shards_idx.second)
          {
            Nodeitem &node_info = m_Node_info[stripe_info.nodes[shard_idx]];
            shards_to_read_for_help_az.push_back({{node_info.Node_ip, node_info.Node_port}, shard_idx});
          }
          shards_to_read.push_back(shards_to_read_for_help_az);
        }
      }
      else if (stripe_info.encodetype == Azure_LRC_1)
      {
        if (failed_shard_idx == (k + g + real_l) || (failed_shard_idx >= k && failed_shard_idx <= (k + g - 1)))
        {
          // 全局校验块那个组的块损坏，一定处于同一个AZ
          std::vector<std::pair<std::pair<std::string, int>, int>> temp3;
          for (int i = k; i <= (k + g - 1); i++)
          {
            if (i != failed_shard_idx)
            {
              Nodeitem &node_info = m_Node_info[stripe_info.nodes[i]];
              temp3.push_back({{node_info.Node_ip, node_info.Node_port}, i});
            }
          }
          if (failed_shard_idx != (k + g + real_l))
          {
            Nodeitem &node_info = m_Node_info[stripe_info.nodes[k + g + real_l]];
            temp3.push_back({{node_info.Node_ip, node_info.Node_port}, k + g + real_l});
          }
          shards_to_read.push_back(temp3);
        }
        else
        {
          // 数据块所在的组损坏，可能波及多个AZ
          int group_idx;
          if (failed_shard_idx >= 0 && failed_shard_idx <= (k - 1))
          {
            group_idx = failed_shard_idx / b;
          }
          else
          {
            group_idx = failed_shard_idx - (k + g);
          }
          std::vector<std::pair<int, int>> live_shards_in_group;
          for (int i = 0; i < b; i++)
          {
            int idx = group_idx * b + i;
            if (idx == failed_shard_idx)
            {
              continue;
            }
            if (idx >= k)
            {
              break;
            }
            live_shards_in_group.push_back({stripe_info.nodes[idx], idx});
          }
          if (k + g + group_idx != failed_shard_idx)
          {
            live_shards_in_group.push_back({stripe_info.nodes[k + g + group_idx], k + g + group_idx});
          }
          std::unordered_set<int> live_shards_group_span_az;
          for (auto &shard : live_shards_in_group)
          {
            live_shards_group_span_az.insert(m_Node_info[shard.first].AZ_id);
          }
          for (auto &az_id : live_shards_group_span_az)
          {
            if (az_id != main_az_id)
            {
              repair_span_az.push_back(az_id);
            }
          }
          for (auto &az_id : repair_span_az)
          {
            std::vector<std::pair<std::pair<std::string, int>, int>> temp4;
            for (auto &live_shard : live_shards_in_group)
            {
              Nodeitem &node_info = m_Node_info[live_shard.first];
              if (node_info.AZ_id == az_id)
              {
                temp4.push_back({{node_info.Node_ip, node_info.Node_port}, live_shard.second});
              }
            }
            shards_to_read.push_back(temp4);
          }
        }
      }
      else if (stripe_info.encodetype == OPPO_LRC)
      {
        // OPPO_LRC单块错误只会出现在1个AZ中，但这牺牲了容错性，在某些参数下无法保证单AZ故障可修复
        std::vector<std::pair<std::pair<std::string, int>, int>> temp5;
        int group_idx;
        std::vector<int> g_num_per_az(real_l, 0);
        int idx = 0;
        for (int i = 0; i < g; i++)
        {
          idx = idx % g_num_per_az.size();
          g_num_per_az[idx++]++;
        }
        if (failed_shard_idx >= 0 && failed_shard_idx <= (k - 1))
        {
          group_idx = failed_shard_idx / b;
        }
        else if (failed_shard_idx >= k && failed_shard_idx <= (k + g - 1))
        {
          int sum = 0;
          for (int i = 0; i < real_l; i++)
          {
            if (k + sum + g_num_per_az[i] - 1 >= failed_shard_idx)
            {
              group_idx = i;
              break;
            }
            sum += g_num_per_az[i];
          }
        }
        else
        {
          group_idx = failed_shard_idx - (k + g);
        }
        for (int i = 0; i < b; i++)
        {
          int idx = group_idx * b + i;
          if (idx == failed_shard_idx)
          {
            continue;
          }
          if (idx >= k)
          {
            break;
          }
          Nodeitem &node_info = m_Node_info[stripe_info.nodes[idx]];
          temp5.push_back({{node_info.Node_ip, node_info.Node_port}, idx});
        }
        int g_start = 0;
        g_start += k;
        for (int i = 0; i < group_idx; i++)
        {
          g_start += g_num_per_az[i];
        }
        for (int j = 0, i = g_start; j < g_num_per_az[group_idx]; j++, i++)
        {
          if (i == failed_shard_idx)
          {
            continue;
          }
          Nodeitem &node_info = m_Node_info[stripe_info.nodes[i]];
          temp5.push_back({{node_info.Node_ip, node_info.Node_port}, i});
        }
        if (k + g + group_idx != failed_shard_idx)
        {
          Nodeitem &node_info = m_Node_info[stripe_info.nodes[k + g + group_idx]];
          temp5.push_back({{node_info.Node_ip, node_info.Node_port}, k + g + group_idx});
        }
        shards_to_read.push_back(temp5);
      }
    }
    else
    {
      std::unordered_map<int, std::vector<int>> live_shards_in_each_az;
      for (int i = 0; i < int(stripe_info.nodes.size()); i++)
      {
        auto lookup = std::find(failed_shard_idxs.begin(), failed_shard_idxs.end(), i);
        // 排除局部校验块
        if (lookup == failed_shard_idxs.end())
        {
          if (stripe_info.encodetype == RS || stripe_info.encodetype == OPPO_LRC)
          {
            if (i < (k + g))
            {
              Nodeitem &node_info = m_Node_info[stripe_info.nodes[i]];
              live_shards_in_each_az[node_info.AZ_id].push_back(i);
            }
          }
          else
          {
            Nodeitem &node_info = m_Node_info[stripe_info.nodes[i]];
            live_shards_in_each_az[node_info.AZ_id].push_back(i);
          }
        }
      }
      std::unordered_map<int, std::vector<int>> failed_shards_in_each_az_with_local;
      for (int i = 0; i < int(failed_shard_idxs.size()); i++)
      {
        Nodeitem &node_info = m_Node_info[stripe_info.nodes[failed_shard_idxs[i]]];
        failed_shards_in_each_az_with_local[node_info.AZ_id].push_back(failed_shard_idxs[i]);
      }
      // 给坏掉的节点寻找新节点
      for (auto &p : failed_shards_in_each_az_with_local)
      {
        int az_id = p.first;
        AZitem &az_info = m_AZ_info[az_id];
        int idx = 0;
        for (int i = 0; i < int(az_info.nodes.size()); i++)
        {
          if (idx >= int(p.second.size()))
          {
            break;
          }
          int node_id = az_info.nodes[i];
          auto lookup = std::find(stripe_info.nodes.begin(), stripe_info.nodes.end(), node_id);
          if (lookup == stripe_info.nodes.end())
          {
            new_locations_with_shard_idx.push_back({node_id, p.second[idx++]});
          }
        }
      }
      if (m_encode_parameter.partial_decoding == false)
      {
        std::vector<std::pair<int, std::vector<int>>> sorted_live_shards_in_each_az;
        for (auto &p : live_shards_in_each_az)
        {
          sorted_live_shards_in_each_az.push_back({p.first, p.second});
        }
        // 按照az中存活块的数量排序
        std::sort(sorted_live_shards_in_each_az.begin(), sorted_live_shards_in_each_az.end(), cmp_num_live_shards);
        int count_shards = 0;
        int expect_shards_number = (stripe_info.encodetype == Azure_LRC_1) ? (k + 1) : (k);
        for (int i = 0; i < int(sorted_live_shards_in_each_az.size()); i++)
        {
          std::vector<std::pair<std::pair<std::string, int>, int>> temp;
          for (int j = 0; j < int(sorted_live_shards_in_each_az[i].second.size()); j++)
          {
            int shard_index = sorted_live_shards_in_each_az[i].second[j];
            if (shard_index != (k + g + real_l))
            {
              Nodeitem &node_info = m_Node_info[stripe_info.nodes[shard_index]];
              temp.push_back({{node_info.Node_ip, node_info.Node_port}, shard_index});
              count_shards++;
              if (count_shards == expect_shards_number)
              {
                break;
              }
            }
          }
          if (!temp.empty())
          {
            shards_to_read.push_back(temp);
            repair_span_az.push_back(sorted_live_shards_in_each_az[i].first);
          }
          if (count_shards == expect_shards_number)
          {
            break;
          }
        }
      }
      else
      {
        int num_failed_shards = 0;
        // RS和OPPO_LRC的num_failed_shards没有局部校验块
        std::vector<int> failed_data_and_parity;
        int repair_limit_num = (stripe_info.encodetype == Azure_LRC_1) ? (k + g + real_l) : (k + g);
        for (auto &p : failed_shards_in_each_az_with_local)
        {
          for (auto &q : p.second)
          {
            if (q < repair_limit_num)
            {
              num_failed_shards++;
              failed_data_and_parity.push_back(q);
            }
          }
        }
        std::set<int> func_idx;
        for (int i = 0; i < int(failed_data_and_parity.size()); i++)
        {
          if (failed_data_and_parity[i] >= k && failed_data_and_parity[i] < repair_limit_num)
          {
            func_idx.insert(failed_data_and_parity[i]);
          }
        }
        for (int i = k; i < repair_limit_num; i++)
        {
          if (func_idx.size() == failed_data_and_parity.size())
          {
            break;
          }
          func_idx.insert(i);
        }
        for (auto &p : live_shards_in_each_az)
        {
          int az_id = p.first;
          std::vector<std::pair<std::pair<std::string, int>, int>> temp;
          for (auto &q : p.second)
          {
            if (q >= k && func_idx.count(q) == 0)
            {
              continue;
            }
            Nodeitem &node_info = m_Node_info[stripe_info.nodes[q]];
            temp.push_back({{node_info.Node_ip, node_info.Node_port}, q});
          }
          if (temp.size() > 0)
          {
            shards_to_read.push_back(temp);
            repair_span_az.push_back(az_id);
            merge[az_id] = (temp.size() >= num_failed_shards);
          }
        }
      }
    }
    return;
  }

  void CoordinatorImpl::do_repair(int stripe_id, std::vector<int> failed_shard_idxs)
  {
    StripeItem &stripe_info = m_Stripe_info[stripe_id];
    if (failed_shard_idxs.size() == 1)
    {
      // 以下两个变量指示了修复流程涉及的AZ以及需要从每个AZ中读取的块
      // 第1个az是main az
      std::vector<std::vector<std::pair<std::pair<std::string, int>, int>>> shards_to_read;
      std::vector<int> repair_span_az;
      // 保存了修复后的shard应该存放的位置及其shard_idx
      std::vector<std::pair<int, int>> new_locations_with_shard_idx;
      // useless here
      std::unordered_map<int, bool> merge;
      generate_repair_plan(stripe_id, true, failed_shard_idxs, shards_to_read, repair_span_az, new_locations_with_shard_idx, merge);
      int main_az_id = repair_span_az[0];
      bool multi_az = (repair_span_az.size() > 1);

      std::vector<std::thread> repairs;
      for (int i = 0; i < int(repair_span_az.size()); i++)
      {
        int az_id = repair_span_az[i];
        if (i == 0)
        {
          // 第1个az是main az
          repairs.push_back(std::thread([&, i, az_id]()
                                        {
          grpc::ClientContext context;
          proxy_proto::mainRepairPlan request;
          proxy_proto::mainRepairReply reply;
          request.set_encode_type(int(stripe_info.encodetype));
          request.set_one_shard_fail(true);
          request.set_multi_az(multi_az);
          request.set_k(stripe_info.k);
          request.set_real_l(stripe_info.real_l);
          request.set_g(stripe_info.g_m);
          request.set_b(stripe_info.b);
          request.set_if_partial_decoding(m_encode_parameter.partial_decoding);
          for (int j = 0; j < int(shards_to_read[0].size()); j++) {
            request.add_inner_az_help_shards_ip(shards_to_read[0][j].first.first);
            request.add_inner_az_help_shards_port(shards_to_read[0][j].first.second);
            request.add_inner_az_help_shards_idx(shards_to_read[0][j].second);
          }
          for (int j = 0; j < int(new_locations_with_shard_idx.size()); j++) {
            Nodeitem &node_info = m_Node_info[new_locations_with_shard_idx[j].first];
            request.add_new_location_ip(node_info.Node_ip);
            request.add_new_location_port(node_info.Node_port);
            request.add_new_location_shard_idx(new_locations_with_shard_idx[j].second);
          }
          request.set_self_az_id(az_id);
          for (auto &help_az_id : repair_span_az) {
            if (help_az_id != main_az_id) {
              request.add_help_azs_id(help_az_id);
            }
          }
          request.set_shard_size(stripe_info.shard_size);
          request.set_stripe_id(stripe_id);
          std::string main_ip_port = m_AZ_info[main_az_id].proxy_ip + ":" + std::to_string(m_AZ_info[main_az_id].proxy_port);
          m_proxy_ptrs[main_ip_port]->mainRepair(&context, request, &reply); }));
        }
        else
        {
          repairs.push_back(std::thread([&, i, az_id]()
                                        {
          grpc::ClientContext context;
          proxy_proto::helpRepairPlan request;
          proxy_proto::helpRepairReply reply;
          request.set_encode_type(int(stripe_info.encodetype));
          request.set_one_shard_fail(true);
          request.set_multi_az(multi_az);
          request.set_k(stripe_info.k);
          request.set_real_l(stripe_info.real_l);
          request.set_g(stripe_info.g_m);
          request.set_b(stripe_info.b);
          request.set_if_partial_decoding(m_encode_parameter.partial_decoding);
          for (int j = 0; j < int(shards_to_read[i].size()); j++) {
            request.add_inner_az_help_shards_ip(shards_to_read[i][j].first.first);
            request.add_inner_az_help_shards_port(shards_to_read[i][j].first.second);
            request.add_inner_az_help_shards_idx(shards_to_read[i][j].second);
          }
          request.set_shard_size(stripe_info.shard_size);
          request.set_main_proxy_ip(m_AZ_info[main_az_id].proxy_ip);
          request.set_main_proxy_port(m_AZ_info[main_az_id].proxy_port + 1);
          request.set_stripe_id(stripe_id);
          request.set_failed_shard_idx(failed_shard_idxs[0]);
          request.set_self_az_id(az_id);
          std::string help_ip_port = m_AZ_info[az_id].proxy_ip + ":" + std::to_string(m_AZ_info[az_id].proxy_port);          
          m_proxy_ptrs[help_ip_port]->helpRepair(&context, request, &reply); }));
        }
      }
      for (int i = 0; i < int(repairs.size()); i++)
      {
        repairs[i].join();
      }
      m_Stripe_info[stripe_id].nodes[new_locations_with_shard_idx[0].second] = new_locations_with_shard_idx[0].first;
    }
    else
    {
      // 以下两个变量指示了修复流程涉及的AZ以及需要从每个AZ中读取的块
      // 第1个az是main az
      std::vector<std::vector<std::pair<std::pair<std::string, int>, int>>> shards_to_read;
      std::vector<int> repair_span_az;
      // 保存了修复后的shard应该存放的位置及其shard_idx
      std::vector<std::pair<int, int>> new_locations_with_shard_idx;
      // help az是否要进行merge操作
      std::unordered_map<int, bool> merge;
      generate_repair_plan(stripe_id, false, failed_shard_idxs, shards_to_read, repair_span_az, new_locations_with_shard_idx, merge);
      int main_az_id = repair_span_az[0];
      bool multi_az = (repair_span_az.size() > 1);
      std::vector<std::thread> repairs;
      for (int i = 0; i < int(repair_span_az.size()); i++)
      {
        int az_id = repair_span_az[i];
        if (i == 0)
        {
          repairs.push_back(std::thread([&, i, az_id]()
                                        {
          grpc::ClientContext context;
          proxy_proto::mainRepairPlan request;
          proxy_proto::mainRepairReply reply;
          request.set_encode_type(int(stripe_info.encodetype));
          request.set_one_shard_fail(false);
          request.set_multi_az(multi_az);
          request.set_k(stripe_info.k);
          request.set_real_l(stripe_info.real_l);
          request.set_g(stripe_info.g_m);
          request.set_b(stripe_info.b);
          request.set_if_partial_decoding(m_encode_parameter.partial_decoding);
          for (int j = 0; j < int(shards_to_read[0].size()); j++) {
            request.add_inner_az_help_shards_ip(shards_to_read[0][j].first.first);
            request.add_inner_az_help_shards_port(shards_to_read[0][j].first.second);
            request.add_inner_az_help_shards_idx(shards_to_read[0][j].second);
          }
          for (int j = 0; j < int(new_locations_with_shard_idx.size()); j++) {
            Nodeitem &node_info = m_Node_info[new_locations_with_shard_idx[j].first];
            request.add_new_location_ip(node_info.Node_ip);
            request.add_new_location_port(node_info.Node_port);
            request.add_new_location_shard_idx(new_locations_with_shard_idx[j].second);
          }
          request.set_self_az_id(az_id);
          for (auto &help_az_id : repair_span_az) {
            if (help_az_id != main_az_id) {
              request.add_help_azs_id(help_az_id);
              if (m_encode_parameter.partial_decoding) {
                request.add_merge(merge[help_az_id]);
              }
            }
          }
          request.set_shard_size(stripe_info.shard_size);
          request.set_stripe_id(stripe_id);
          for (auto p : failed_shard_idxs) {
            request.add_all_failed_shards_idx(p);
          }
          std::string main_ip_port = m_AZ_info[main_az_id].proxy_ip + ":" + std::to_string(m_AZ_info[main_az_id].proxy_port);
          m_proxy_ptrs[main_ip_port]->mainRepair(&context, request, &reply); }));
        }
        else
        {
          repairs.push_back(std::thread([&, i, az_id]()
                                        {
          grpc::ClientContext context;
          proxy_proto::helpRepairPlan request;
          proxy_proto::helpRepairReply reply;
          request.set_encode_type(int(stripe_info.encodetype));
          request.set_one_shard_fail(false);
          request.set_multi_az(multi_az);
          request.set_k(stripe_info.k);
          request.set_real_l(stripe_info.real_l);
          request.set_g(stripe_info.g_m);
          request.set_b(stripe_info.b);
          request.set_if_partial_decoding(m_encode_parameter.partial_decoding);
          for (int j = 0; j < int(shards_to_read[i].size()); j++) {
            request.add_inner_az_help_shards_ip(shards_to_read[i][j].first.first);
            request.add_inner_az_help_shards_port(shards_to_read[i][j].first.second);
            request.add_inner_az_help_shards_idx(shards_to_read[i][j].second);
          }
          request.set_shard_size(stripe_info.shard_size);
          request.set_main_proxy_ip(m_AZ_info[main_az_id].proxy_ip);
          request.set_main_proxy_port(m_AZ_info[main_az_id].proxy_port + 1);
          request.set_stripe_id(stripe_id);
          request.set_failed_shard_idx(failed_shard_idxs[0]);
          request.set_self_az_id(az_id);
          if (m_encode_parameter.partial_decoding) {
            request.set_merge(merge[az_id]);
          }
          for (auto p : failed_shard_idxs) {
            request.add_all_failed_shards_idx(p);
          }
          std::string help_ip_port = m_AZ_info[az_id].proxy_ip + ":" + std::to_string(m_AZ_info[az_id].proxy_port);
          m_proxy_ptrs[help_ip_port]->helpRepair(&context, request, &reply); }));
        }
      }
      for (int i = 0; i < int(repairs.size()); i++)
      {
        repairs[i].join();
      }
      int expect_new_location_number = (stripe_info.encodetype == Azure_LRC_1) ? (stripe_info.k + stripe_info.g_m + stripe_info.real_l) : (stripe_info.k + stripe_info.g_m);
      for (auto &p : new_locations_with_shard_idx)
      {
        if (p.second < expect_new_location_number)
        {
          m_Stripe_info[stripe_id].nodes[p.second] = p.first;
        }
      }
    }
  }

  bool CoordinatorImpl::init_proxy(std::string proxy_information_path)
  {
    /*需要补充修改，这里需要读取.xml的proxy的ip来初始化，
    将proxy的_stub初始化到m_proxy_ptrs中*/
    /*配置文件的路径是proxy_information_path*/

    /*没必要再读取配置文件了*/
    for (auto cur = m_AZ_info.begin(); cur != m_AZ_info.end(); cur++)
    {
      std::string proxy_ip_and_port = cur->second.proxy_ip + ":" + std::to_string(cur->second.proxy_port);
      auto _stub = proxy_proto::proxyService::NewStub(grpc::CreateChannel(proxy_ip_and_port, grpc::InsecureChannelCredentials()));
      proxy_proto::CheckaliveCMD Cmd;
      proxy_proto::RequestResult result;
      grpc::ClientContext clientContext;
      Cmd.set_name("wwwwwwwww");
      grpc::Status status;
      status = _stub->checkalive(&clientContext, Cmd, &result);
      if (status.ok())
      {
        std::cout << "checkalive,ok" << std::endl;
      }else{
        std::cout << "checkalive,fail" << std::endl;
      }
      m_proxy_ptrs.insert(std::make_pair(proxy_ip_and_port, std::move(_stub)));
    }
    return true;
  }
  bool CoordinatorImpl::init_AZinformation(std::string Azinformation_path)
  {

    /*需要补充修改，这里需要读取.xml的proxy的ip来初始化，
  将datanode和AZ的信息初始化到m_AZ_info中*/
    /*配置文件的路径是Azinformation_path*/
    std::cout << "Azinformation_path:" << Azinformation_path << std::endl;
    tinyxml2::XMLDocument xml;
    xml.LoadFile(Azinformation_path.c_str());
    tinyxml2::XMLElement *root = xml.RootElement();
    int node_id = 0;
    for (tinyxml2::XMLElement *az = root->FirstChildElement(); az != nullptr; az = az->NextSiblingElement())
    {
      std::string az_id(az->Attribute("id"));
      std::string proxy(az->Attribute("proxy"));
      std::cout << "az_id: " << az_id << " , proxy: " << proxy << std::endl;
      m_AZ_info[std::stoi(az_id)].AZ_id = std::stoi(az_id);
      auto pos = proxy.find(':');
      m_AZ_info[std::stoi(az_id)].proxy_ip = proxy.substr(0, pos);
      m_AZ_info[std::stoi(az_id)].proxy_port = std::stoi(proxy.substr(pos + 1, proxy.size()));
      for (tinyxml2::XMLElement *node = az->FirstChildElement()->FirstChildElement(); node != nullptr; node = node->NextSiblingElement())
      {
        std::string node_uri(node->Attribute("uri"));
        std::cout << "____node: " << node_uri << std::endl;
        m_AZ_info[std::stoi(az_id)].nodes.push_back(node_id);
        m_Node_info[node_id].Node_id = node_id;
        auto pos = node_uri.find(':');
        m_Node_info[node_id].Node_ip = node_uri.substr(0, pos);
        m_Node_info[node_id].Node_port = std::stoi(node_uri.substr(pos + 1, node_uri.size()));
        m_Node_info[node_id].AZ_id = std::stoi(az_id);
        node_id++;
      }
    }
    return true;
  }
  void CoordinatorImpl::generate_placement(std::vector<unsigned int> &stripe_nodes, int stripe_id)
  {
    // Flat以后再说

    StripeItem &stripe_info = m_Stripe_info[stripe_id];
    int k = stripe_info.k;
    int real_l = stripe_info.real_l;
    int full_group_l = (k % real_l != 0) ? (real_l - 1) : real_l;
    int g_m = stripe_info.g_m;
    int b = stripe_info.b;
    int tail_group = k - full_group_l * b;
    OppoProject::EncodeType encode_type = stripe_info.encodetype;
    OppoProject::PlacementType placement_type = m_encode_parameter.placementtype;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned int> dis(0, m_Node_info.size() - 1);

    if (encode_type == RS)
    {
      // RS只能使用Flat或者Random
      // stripe_nodes依次对应k个数据块、g个校验块的放置节点
      if (placement_type == Random)
      {
        std::vector<bool> vis(m_Node_info.size(), false);
        std::vector<int> num_chosen_nodes_per_az(m_AZ_info.size(), 0);
        for (int i = 0; i < k + g_m; i++)
        {
          int node_idx;
          do
          {
            node_idx = dis(gen);
            // 每个AZ内不能放置超过k个块，以保证单AZ可修复
          } while (vis[node_idx] == true || num_chosen_nodes_per_az[m_Node_info[node_idx].AZ_id] == k);
          stripe_nodes.push_back(node_idx);
          m_Node_info[node_idx].stripes.insert(stripe_id);
          vis[node_idx] = true;
          num_chosen_nodes_per_az[m_Node_info[node_idx].AZ_id]++;
        }
      }
    }
    else if (encode_type == Azure_LRC_1)
    {
      // stripe_nodes依次对应k个数据块、g个全局校验块、l+1个局部校验块的放置节点
      // 且k个数据块被分为了l个组，每个组在stripe_nodes中是连续的
      if (placement_type == Random)
      {
        std::vector<bool> vis(m_Node_info.size(), false);
        std::vector<std::pair<std::unordered_set<int>, int>> help(m_AZ_info.size());
        for (int i = 0; i < int(m_AZ_info.size()); i++)
        {
          help[i].second = 0;
        }
        int node_idx, az_idx, area_upper;
        for (int i = 0; i < full_group_l; i++)
        {
          for (int j = 0; j < b; j++)
          {
            do
            {
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
        for (int i = 0; i < tail_group; i++)
        {
          do
          {
            node_idx = dis(gen);
            az_idx = m_Node_info[node_idx].AZ_id;
            area_upper = g_m + help[az_idx].first.size();
          } while (vis[node_idx] == true || help[az_idx].second == area_upper);
          stripe_nodes.push_back(node_idx);
          m_Node_info[node_idx].stripes.insert(stripe_id);
          vis[node_idx] = true;
          help[az_idx].first.insert(full_group_l);
          help[az_idx].second++;
        }
        for (int i = 0; i < g_m; i++)
        {
          do
          {
            node_idx = dis(gen);
            az_idx = m_Node_info[node_idx].AZ_id;
            area_upper = g_m + help[az_idx].first.size();
          } while (vis[node_idx] == true || help[az_idx].second == area_upper);
          stripe_nodes.push_back(node_idx);
          m_Node_info[node_idx].stripes.insert(stripe_id);
          vis[node_idx] = true;
          help[az_idx].second++;
        }
        for (int i = 0; i < full_group_l; i++)
        {
          do
          {
            node_idx = dis(gen);
            az_idx = m_Node_info[node_idx].AZ_id;
            area_upper = g_m + help[az_idx].first.size();
          } while (vis[node_idx] == true || help[az_idx].second == area_upper);
          stripe_nodes.push_back(node_idx);
          m_Node_info[node_idx].stripes.insert(stripe_id);
          vis[node_idx] = true;
          if (help[az_idx].first.count(i) == 0)
          {
            help[az_idx].first.insert(i);
          }
          help[az_idx].second++;
        }
        if (tail_group > 0)
        {
          do
          {
            node_idx = dis(gen);
            az_idx = m_Node_info[node_idx].AZ_id;
            area_upper = g_m + help[az_idx].first.size();
          } while (vis[node_idx] == true || help[az_idx].second == area_upper);
          stripe_nodes.push_back(node_idx);
          m_Node_info[node_idx].stripes.insert(stripe_id);
          vis[node_idx] = true;
          if (help[az_idx].first.count(full_group_l) == 0)
          {
            help[az_idx].first.insert(full_group_l);
          }
          help[az_idx].second++;
        }
        // 最后还有1个由全局校验块生成的局部校验块
        do
        {
          node_idx = dis(gen);
          az_idx = m_Node_info[node_idx].AZ_id;
          area_upper = g_m + help[az_idx].first.size();
        } while (vis[node_idx] == true || help[az_idx].second == area_upper);
        stripe_nodes.push_back(node_idx);
        m_Node_info[node_idx].stripes.insert(stripe_id);
        vis[node_idx] = true;
        help[az_idx].second++;
      }
      else if (placement_type == Best_Placement)
      {
        int start_idx = 0;
        int sita = g_m / b;
        int num_nodes = tail_group > 0 ? (k + g_m + full_group_l + 1 + 1) : (k + g_m + full_group_l + 1);
        stripe_nodes.resize(num_nodes);
        if (sita >= 1)
        {
          int left_data_shard = k;
          while (left_data_shard > 0)
          {
            if (left_data_shard >= sita * b)
            {
              cur_az = cur_az % m_AZ_info.size();
              AZitem &az = m_AZ_info[cur_az++];
              for (int i = 0; i < sita * b; i++)
              {
                az.cur_node = az.cur_node % az.nodes.size();
                stripe_nodes[start_idx + i] = az.nodes[az.cur_node++];
                m_Node_info[az.nodes[az.cur_node - 1]].stripes.insert(stripe_id);
              }
              for (int i = 0; i < sita; i++)
              {
                az.cur_node = az.cur_node % az.nodes.size();
                stripe_nodes[k + g_m + start_idx / b + i] = az.nodes[az.cur_node++];
                m_Node_info[az.nodes[az.cur_node - 1]].stripes.insert(stripe_id);
              }
              start_idx += (sita * b);
              left_data_shard -= (sita * b);
            }
            else
            {
              int left_group = left_data_shard / b;
              cur_az = cur_az % m_AZ_info.size();
              AZitem &az = m_AZ_info[cur_az++];
              for (int i = 0; i < left_group * b; i++)
              {
                az.cur_node = az.cur_node % az.nodes.size();
                stripe_nodes[start_idx + i] = az.nodes[az.cur_node++];
                m_Node_info[az.nodes[az.cur_node - 1]].stripes.insert(stripe_id);
              }
              for (int i = 0; i < left_group; i++)
              {
                az.cur_node = az.cur_node % az.nodes.size();
                stripe_nodes[k + g_m + start_idx / b + i] = az.nodes[az.cur_node++];
                m_Node_info[az.nodes[az.cur_node - 1]].stripes.insert(stripe_id);
              }
              start_idx += (left_group * b);
              left_data_shard -= (left_group * b);
              if (left_data_shard > 0)
              {
                for (int i = 0; i < left_data_shard; i++)
                {
                  az.cur_node = az.cur_node % az.nodes.size();
                  stripe_nodes[start_idx + i] = az.nodes[az.cur_node++];
                  m_Node_info[az.nodes[az.cur_node - 1]].stripes.insert(stripe_id);
                }
                az.cur_node = az.cur_node % az.nodes.size();
                stripe_nodes[k + g_m + start_idx / b] = az.nodes[az.cur_node++];
                m_Node_info[az.nodes[az.cur_node - 1]].stripes.insert(stripe_id);
                left_data_shard -= left_data_shard;
              }
            }
          }
          cur_az = cur_az % m_AZ_info.size();
          AZitem &az = m_AZ_info[cur_az++];
          for (int i = 0; i < g_m; i++)
          {
            az.cur_node = az.cur_node % az.nodes.size();
            stripe_nodes[k + i] = az.nodes[az.cur_node++];
            m_Node_info[az.nodes[az.cur_node - 1]].stripes.insert(stripe_id);
          }
          az.cur_node = az.cur_node % az.nodes.size();
          stripe_nodes[stripe_nodes.size() - 1] = az.nodes[az.cur_node++];
          m_Node_info[az.nodes[az.cur_node - 1]].stripes.insert(stripe_id);
        }
        else
        {
          int idx = 0;
          for (int i = 0; i <= full_group_l; i++)
          {
            int left_data_shard_in_group = b;
            if (i == full_group_l)
            {
              if (tail_group <= 0)
              {
                continue;
              }
            }
            if (i == full_group_l)
            {
              left_data_shard_in_group = tail_group;
            }
            while (left_data_shard_in_group >= g_m + 1)
            {
              cur_az = cur_az % m_AZ_info.size();
              AZitem &az = m_AZ_info[cur_az++];
              for (int j = 0; j < g_m + 1; j++)
              {
                az.cur_node = az.cur_node % az.nodes.size();
                stripe_nodes[idx++] = az.nodes[az.cur_node++];
                m_Node_info[az.nodes[az.cur_node - 1]].stripes.insert(stripe_id);
              }
              left_data_shard_in_group -= (g_m + 1);
            }
            if (left_data_shard_in_group == 0)
            {
              cur_az = cur_az % m_AZ_info.size();
              AZitem &az = m_AZ_info[cur_az++];
              az.cur_node = az.cur_node % az.nodes.size();
              stripe_nodes[k + g_m + i] = az.nodes[az.cur_node++];
              m_Node_info[az.nodes[az.cur_node - 1]].stripes.insert(stripe_id);
              continue;
            }
            cur_az = cur_az % m_AZ_info.size();
            AZitem &az = m_AZ_info[cur_az++];
            for (int i = 0; i < left_data_shard_in_group; i++)
            {
              az.cur_node = az.cur_node % az.nodes.size();
              stripe_nodes[idx++] = az.nodes[az.cur_node++];
              m_Node_info[az.nodes[az.cur_node - 1]].stripes.insert(stripe_id);
            }
            az.cur_node = az.cur_node % az.nodes.size();
            stripe_nodes[k + g_m + i] = az.nodes[az.cur_node++];
            m_Node_info[az.nodes[az.cur_node - 1]].stripes.insert(stripe_id);
          }
          cur_az = cur_az % m_AZ_info.size();
          AZitem &az = m_AZ_info[cur_az++];
          for (int i = 0; i < g_m; i++)
          {
            az.cur_node = az.cur_node % az.nodes.size();
            stripe_nodes[k + i] = az.nodes[az.cur_node++];
            m_Node_info[az.nodes[az.cur_node - 1]].stripes.insert(stripe_id);
          }
          az.cur_node = az.cur_node % az.nodes.size();
          stripe_nodes[stripe_nodes.size() - 1] = az.nodes[az.cur_node++];
          m_Node_info[az.nodes[az.cur_node - 1]].stripes.insert(stripe_id);
        }
      }
    }
    else if (encode_type == OPPO_LRC)
    {
      // OPPO_LRC1个group放1个az
      std::vector<int> az_ids;
      for (int i = 0; i < real_l; i++)
      {
        cur_az = cur_az % m_AZ_info.size();
        AZitem &az = m_AZ_info[cur_az++];
        az_ids.push_back(az.AZ_id);
      }
      for (int i = 0; i < full_group_l; i++)
      {
        AZitem &az = m_AZ_info[az_ids[i]];
        for (int j = 0; j < b; j++)
        {
          az.cur_node = az.cur_node % az.nodes.size();
          stripe_nodes.push_back(az.nodes[az.cur_node++]);
          m_Node_info[az.nodes[az.cur_node - 1]].stripes.insert(stripe_id);
        }
      }
      if (tail_group > 0)
      {
        AZitem &az = m_AZ_info[az_ids[full_group_l]];
        for (int i = 0; i < tail_group; i++)
        {
          az.cur_node = az.cur_node % az.nodes.size();
          stripe_nodes.push_back(az.nodes[az.cur_node++]);
          m_Node_info[az.nodes[az.cur_node - 1]].stripes.insert(stripe_id);
        }
      }
      std::vector<int> g_num_per_az(az_ids.size(), 0);
      int idx = 0;
      for (int i = 0; i < g_m; i++)
      {
        idx = idx % g_num_per_az.size();
        g_num_per_az[idx++]++;
      }
      for (int i = 0; i < int(az_ids.size()); i++)
      {
        AZitem &az = m_AZ_info[az_ids[i]];
        for (int j = 0; j < g_num_per_az[i]; j++)
        {
          az.cur_node = az.cur_node % az.nodes.size();
          stripe_nodes.push_back(az.nodes[az.cur_node++]);
          m_Node_info[az.nodes[az.cur_node - 1]].stripes.insert(stripe_id);
        }
      }
      for (int i = 0; i < int(az_ids.size()); i++)
      {
        AZitem &az = m_AZ_info[az_ids[i]];
        az.cur_node = az.cur_node % az.nodes.size();
        stripe_nodes.push_back(az.nodes[az.cur_node++]);
        m_Node_info[az.nodes[az.cur_node - 1]].stripes.insert(stripe_id);
      }
    }
    // for (int i = 0; i < full_group_l; i++) {
    //   for (int j = 0; j < b; j++) {
    //     std::cout << stripe_nodes[i * b + j] << " ";
    //   }
    //   std::cout << std::endl;
    // }
    // if (tail_group > 0) {
    //   for (int i = 0; i < tail_group; i++) {
    //     std::cout << stripe_nodes[full_group_l * b + i] << " ";
    //   }
    //   std::cout << std::endl;
    // }
    // for (int i = 0; i < g_m; i++) {
    //   std::cout << stripe_nodes[k + i] << " ";
    // }
    // std::cout << std::endl;
    // int real_l_include_gl = tail_group > 0 ? (full_group_l + 1 + 1) : (full_group_l + 1);
    // for (int i = 0; i < real_l_include_gl; i++) {
    //   std::cout << stripe_nodes[k + g_m + i] << " ";
    // }
    // std::cout << std::endl;
    // std::cout << "******************************" << std::endl;
    return;
  }

  // update
  // Cross AZ aware
  grpc::Status CoordinatorImpl::updateGetLocation(::grpc::ServerContext *context,
                                                  const coordinator_proto::UpdatePrepareRequest *request,
                                                  coordinator_proto::UpdateDataLocation *data_location)
  {
    std::string key = request->key();
    int update_offset_infile = request->offset();
    int update_length = request->length();

    try
    {
      auto updated_stripe_shards = split_update_length(key, update_offset_infile, update_length);
      if (updated_stripe_shards.size() > 1)
        std::cerr << "to much updated stripes" << std::endl;
      auto it = updated_stripe_shards.begin();
      unsigned int temp_stripe_id = it->first;
      StripeItem &temp_stripe = m_Stripe_info[temp_stripe_id];
      auto idx_ranges = it->second;
      std::map<int, std::vector<ShardidxRange>> AZ_updated_idxrange;
      std::map<int, std::vector<int>> AZ_global_parity_idx;
      std::map<int, std::vector<int>> AZ_local_parity_idx;
      for (int i = 0; i < idx_ranges.size(); i++)
      {
        int shardidx = idx_ranges[i].shardidx;
        Nodeitem &tempnode = m_Node_info[temp_stripe.nodes[shardidx]];
        int AZid = tempnode.AZ_id;
        AZ_updated_idxrange[AZid].push_back(idx_ranges[i]);
      }
      for (int i = temp_stripe.k; i < temp_stripe.k + temp_stripe.g_m; i++)
      {
        Nodeitem &tempnode = m_Node_info[temp_stripe.nodes[i]];
        int AZid = tempnode.AZ_id;
        AZ_global_parity_idx[AZid].push_back(i);
      }
      if (temp_stripe.encodetype == Azure_LRC_1 || temp_stripe.encodetype == OPPO_LRC)
      {
        for (int i = temp_stripe.k + temp_stripe.g_m; i < temp_stripe.k + temp_stripe.g_m + temp_stripe.real_l; i++)
        {
          Nodeitem &tempnode = m_Node_info[temp_stripe.nodes[i]];
          int AZid = tempnode.AZ_id;
          AZ_local_parity_idx[AZid].push_back(i);
        }
      }

      int collecor_AZid = -1;

      if (temp_stripe.encodetype == Azure_LRC_1)
      { // other placement to be done
        std::map<int, std::vector<ShardidxRange>>::iterator max_num_idx_iter = AZ_updated_idxrange.begin();
        for (auto it = AZ_updated_idxrange.begin(); it != AZ_updated_idxrange.end(); it++)
        {
          if ((it->second).size() > (max_num_idx_iter->second).size())
            max_num_idx_iter = it;
        }

        auto max_num_global_iter = AZ_global_parity_idx.begin();
        for (auto it = AZ_global_parity_idx.begin(); it != AZ_global_parity_idx.end(); it++)
          if ((it->second).size() > (max_num_global_iter->second).size())
            max_num_global_iter = it;

        collecor_AZid = max_num_global_iter->first;
      }
      else if (temp_stripe.encodetype == OPPO_LRC)
      {
        std::map<int, std::vector<ShardidxRange>>::iterator max_num_idx_iter = AZ_updated_idxrange.begin();
        for (auto it = AZ_updated_idxrange.begin(); it != AZ_updated_idxrange.end(); it++)
        {
          if ((it->second).size() > (max_num_idx_iter->second).size())
            max_num_idx_iter = it;
        }

        auto max_num_global_iter = AZ_global_parity_idx.begin();
        for (auto it = AZ_global_parity_idx.begin(); it != AZ_global_parity_idx.end(); it++)
        {
          int temp_AZid = it->first;
          int max_AZid = max_num_global_iter->first;
          // local pairty is seem as global parity
          int temp_p_num = AZ_global_parity_idx[temp_AZid].size() + AZ_local_parity_idx[temp_p_num].size();
          int max_p_num = AZ_global_parity_idx[max_AZid].size() + AZ_local_parity_idx[max_p_num].size();
          if (temp_p_num > max_p_num)
            max_num_global_iter = it;
        }
        collecor_AZid = max_num_global_iter->first;
      }

      // fill reply to client
      data_location->set_key(key);
      data_location->set_stripeid(temp_stripe.Stripe_id);
      for (auto const &t_item : AZ_updated_idxrange)
      {
        int azid = t_item.first;
        auto t_idxranges = t_item.second;
        data_location->add_proxyip(m_AZ_info[azid].proxy_ip);
        data_location->add_proxyport(m_AZ_info[azid].proxy_port);
        data_location->add_num_each_proxy((int)t_idxranges.size());
        for (int i = 0; i < t_idxranges.size(); i++)
        {
          data_location->add_datashardidx(t_idxranges[i].shardidx);
          data_location->add_offsetinshard(t_idxranges[i].offset_in_shard);
          data_location->add_lengthinshard(t_idxranges[i].range_length);
        }
      }

      // fill notice to dataproxy and collector proxy
      int data_proxy_num = AZ_updated_idxrange.size();
      std::vector<proxy_proto::DataProxyUpdatePlan> dataproxy_notices;
      proxy_proto::CollectorProxyUpdatePlan collector_notice;
      for (auto const &t_item : AZ_updated_idxrange)
      {
        proxy_proto::DataProxyUpdatePlan notice;
        notice.set_key(key);
        notice.set_stripeid(temp_stripe.Stripe_id);

        // data shard
        auto t_idxranges = t_item.second;
        for (int i = 0; i < t_idxranges.size(); i++)
        {
          int idx = t_idxranges[i].shardidx;
          notice.receive_client_shard_idx(idx);
          notice.receive_cross_az_offset_in_shard(t_idxranges[i].offset_in_shard);
          notice.receive_client_shard_length(t_idxranges[i].range_length);
          Nodeitem &tnode = m_Node_info[temp_stripe.nodes[idx]];
          notice.add_data_nodeip(tnode.Node_ip);
          notice.add_data_nodeport(tnode.Node_port);
        }

        // local parity
        int azid = t_item.first;
        auto local_parity_idxes = AZ_local_parity_idx[azid];
        for (auto const &idx : local_parity_idxes)
        {
          notice.add_local_parity_idx(idx);
          Nodeitem &tnode = m_Node_info[temp_stripe.nodes[idx]];
          notice.add_local_parity_nodeip(tnode.Node_ip);
          notice.add_local_parity_nodeport(tnode.Node_port);
        }
        notice.set_collector_proxyip(m_AZ_info[collecor_AZid].proxy_ip);
        notice.set_collector_proxyport(m_AZ_info[collecor_AZid].proxy_port);

        switch (temp_stripe.encodetype)
        {
        case Azure_LRC_1:
          /*to be done */
          break;
        case OPPO_LRC:
          break;
        default:
          break;
        }
        dataproxy_notices.push_back(notice);
      }

      // fill collector notice

      collector_notice.set_key(key);
      collector_notice.set_stripeid(temp_stripe.Stripe_id);
      if (temp_stripe.encodetype == Azure_LRC_1)
      {
        // receive from data proxy
        for (auto const &t_item : AZ_updated_idxrange)
        {
          int azid = t_item.first;
          collector_notice.add_data_proxyip(m_AZ_info[azid].proxy_ip);
          collector_notice.add_data_proxyport(m_AZ_info[azid].proxy_port);
          auto t_idxranges = t_item.second;
          collector_notice.add_idx_num_each_proxy(t_idxranges.size());
          for (int i = 0; i < t_idxranges.size(); i++)
          {
            collector_notice.add_receive_proxy_shard_idx(t_idxranges[i].shardidx);
            collector_notice.add_receive_proxy_shard_offset(t_idxranges[i].offset_in_shard);
            collector_notice.add_receive_proxy_shard_length(t_idxranges[i].range_length);
          }
        }

        // local parity
        auto local_idxes = AZ_local_parity_idx[collecor_AZid];
        for (auto const &idx : local_idxes)
        {
          Nodeitem &tnode = m_Node_info[temp_stripe.nodes[idx]];
          collector_notice.add_local_parity_idx(idx);
          collector_notice.add_local_parity_nodeip(tnode.Node_ip);
          collector_notice.add_local_parity_nodeport(tnode.Node_port);
        }

        // global parity
        auto global_idxes = AZ_global_parity_idx[collecor_AZid];
        for (auto const &idx : global_idxes)
        {
          Nodeitem &tnode = m_Node_info[temp_stripe.nodes[idx]];
          collector_notice.add_global_parity_idx(idx);
          collector_notice.add_global_parity_nodeip(tnode.Node_ip);
          collector_notice.add_global_parity_nodeport(tnode.Node_port);
        }
      }

      m_mutex.lock();
      data_location->set_update_operation_id(m_next_update_opration_id);
      for (int i = 0; i < dataproxy_notices.size(); i++)
        dataproxy_notices[i].set_update_opration_id(m_next_update_opration_id);
      collector_notice.set_update_operation_id(m_next_update_opration_id);
      m_next_update_opration_id++;
      m_mutex.unlock();

      // rpc proxy
    }
    catch (const std::exception &e)
    {
      std::cerr << e.what() << '\n';
    }

    return grpc::Status::OK;
  }

  std::map<unsigned int, std::vector<ShardidxRange>>
  CoordinatorImpl::split_update_length(std::string key, int update_offset_infile, int update_length)
  {

    std::map<unsigned int, std::vector<ShardidxRange>> updated_stripe_shards;

    int update_offset_inshard = -1;
    auto it = m_object_table_big_small_commit.find(key);
    if (it == m_object_table_big_small_commit.end())
    {
      std::cerr << "updated object doesn't exist" << std::endl;
    }
    ObjectItemBigSmall object = m_object_table_big_small_commit[key];
    if (update_offset_infile + update_length > object.object_size)
      std::cerr << "update length too long" << std::endl;

    if (!object.big_object)
    {
      StripeItem stripe = m_Stripe_info[object.stripes[0]];
      update_offset_inshard = object.offset + update_offset_infile;
      updated_stripe_shards[stripe.Stripe_id].push_back(ShardidxRange(object.shard_idx, update_offset_inshard, update_length));
    }
    else
    {
      int desend_offset = update_offset_infile;
      int desend_len = update_length;
      int i = 0; // index of stipe_ids,used later
      std::vector<unsigned int> &stripe_ids = object.stripes;
      for (i = 0; i < stripe_ids.size(); i++)
      {
        StripeItem &temp_stripe = m_Stripe_info[stripe_ids[i]];
        if (desend_offset > temp_stripe.k * temp_stripe.shard_size)
          desend_offset -= temp_stripe.k * temp_stripe.shard_size;
        else
          break;
      }
      if (i >= stripe_ids.size())
        std::cerr << "not valid update offset" << std::endl;

      StripeItem &first_updated_stripe = m_Stripe_info[stripe_ids[i]];
      int first_update_idx = desend_offset / first_updated_stripe.shard_size;
      int first_offset_in_shard = desend_offset - first_update_idx * first_updated_stripe.shard_size;
      int rest_len = first_updated_stripe.shard_size - first_offset_in_shard;
      int first_shard_update_len = 0;
      if (rest_len >= update_length)
      {
        first_shard_update_len = update_length;
        desend_len = 0;
      }
      else
      {
        first_shard_update_len = rest_len;
        desend_len -= first_shard_update_len;
      }
      ShardidxRange first_idx_range(first_update_idx, first_offset_in_shard, first_shard_update_len);
      updated_stripe_shards[first_updated_stripe.Stripe_id].push_back(first_idx_range);
      if (desend_len > 0 && first_update_idx <= first_updated_stripe.k - 2)
      { // other idx of 1st stipe
        int tt_idx = first_update_idx + 1;
        for (; tt_idx < first_updated_stripe.k && desend_len > 0; tt_idx++)
        {
          if (desend_len > first_updated_stripe.shard_size)
          {
            int temp_offset = 0;
            int temp_len = first_updated_stripe.shard_size;
            updated_stripe_shards[first_updated_stripe.Stripe_id].push_back(ShardidxRange(tt_idx, temp_offset, temp_len));
            desend_len -= temp_len;
          }
          else
          {
            int temp_offset = 0;
            int temp_len = desend_len;
            updated_stripe_shards[first_updated_stripe.Stripe_id].push_back(ShardidxRange(tt_idx, temp_offset, temp_len));
            desend_len -= temp_len;
          }
        }
      }
      if (desend_len > 0)
      { // more than 1 stripe
        i++;
        for (; i < stripe_ids.size() && desend_len > 0; i++)
        {
          StripeItem &temp_stripe = m_Stripe_info[stripe_ids[i]];
          for (int tt_idx = 0; tt_idx < temp_stripe.k && desend_len > 0; tt_idx++)
          { // each shard
            if (desend_len > temp_stripe.shard_size)
            {
              int temp_offset = 0;
              int temp_len = temp_stripe.shard_size;
              updated_stripe_shards[temp_stripe.Stripe_id].push_back(ShardidxRange(tt_idx, temp_offset, temp_len));
              desend_len -= temp_len;
            }
            else
            {
              int temp_offset = 0;
              int temp_len = desend_len;
              updated_stripe_shards[temp_stripe.Stripe_id].push_back(ShardidxRange(tt_idx, temp_offset, temp_len));
              desend_len -= temp_len;
            }
          }
        }
      }
    }
    return updated_stripe_shards;
  }

} // namespace OppoProject
