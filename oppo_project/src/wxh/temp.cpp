grpc::Status ProxyImpl::dataProxyUpdate(
      grpc::ServerContext *context,
      const proxy_proto::DataProxyUpdatePlan *dataProxyPlan,
      proxy_proto::DataProxyReply *reply)
  {
    std::string key = dataProxyPlan->key();
    unsigned int stripeid = dataProxyPlan->stripeid();

    int update_op_id = dataProxyPlan->update_opration_id();
    int encode_type=dataProxyPlan->encode_type();

    std::vector<ShardidxRange> data_idx_ranges;//idx->Range 
    std::vector<std::pair<std::string, int> > data_nodes_ip_port;
    std::vector<int> localparity_idxes;
    std::unordered_map<int,std::pair<std::string, int> > localparity_nodes_ip_port;//local idx->node ip port
    std::vector<int> globalparity_idxes;
    std::unordered_map<int,std::pair<std::string, int> > globalparity_nodes_ip_port;
    std::string collector_proxyip = dataProxyPlan->collector_proxyip();
    int collector_proxyport = dataProxyPlan->collector_proxyport();
    
    for (int i = 0; i < dataProxyPlan->receive_client_shard_idx_size(); i++)
    {
      int idx = dataProxyPlan->receive_client_shard_idx(i);
      int offset = dataProxyPlan->receive_client_shard_offset(i);
      int len = dataProxyPlan->receive_client_shard_offset(i);
      //data_idx_ranges[idx] = Range(offset, len);
      data_idx_ranges.push_back(ShardidxRange(idx,offset,len));
      data_nodes_ip_port.push_back(std::make_pair(dataProxyPlan->data_nodeip(i), dataProxyPlan->data_nodeport(i)));
    }
    if(encode_type==1 ||encode_type==2){//OPPO LRC || AzureLRC+1
    }

    // may need to call repair
    

    auto receive_update = [this, key, stripeid, update_op_id, encode_type,data_idx_ranges, data_nodes_ip_port, localparity_idxes, localparity_nodes_ip_port, globalparity_idxes,globalparity_nodes_ip_port,collector_proxyip, collector_proxyport]() mutable
    {
      try
      { 
        //receive from client
        asio::ip::tcp::socket socket_data(io_context);
        acceptor.accept(socket_data);
        asio::error_code error;
        OppoProject::Role sender_role=(OppoProject::Role) OppoProject::receive_int(socket_data,error);
        if(sender_role!=OppoProject::RoleClient) std::cout<<"data proxy receive error"<<std::endl;
        //std::vector<char> v_key(key.size());
        //asio::read(socket_data,asio::buffer(v_key,key.size()),error);

        //std::cout<<"data proxy receive key:"<<v_key.data()<<std::endl;
        
        int client_stripeid = OppoProject::receive_int(socket_data,error);
        int shardidx = OppoProject::receive_int(socket_data,error);

        if(client_stripeid!=stripeid){
          std::cout<<"client stripeid:"<<client_stripeid<<" stripeid from MDS:"<<stripeid<<std::endl;
          std::cerr<<"update stripeid doesn't match (client and coordinator)"<<std::endl;
        }
        
        std::vector<std::vector<char> > new_shard_data_vector;
        std::vector<std::vector<char> > old_shard_data_vector;
        std::vector<std::vector<char> > data_delta_vector;
        
        int i=0;
        for(i=0;i<data_idx_ranges.size();i++){
          std::vector<char> v_data(data_idx_ranges[i].range_length);
          std::vector<char> old_data(data_idx_ranges[i].range_length);
          std::vector<char> data_delta(data_idx_ranges[i].range_length);
          

          asio::read(socket_data,asio::buffer(v_data,v_data.size()), error);
          
          new_shard_data_vector.push_back(v_data);
          old_shard_data_vector.push_back(old_data);//assignment
          data_delta_vector.push_back(data_delta);
        }
        asio::error_code ignore_ec;
        
        
        auto read_from_datanode=[this](std::string shardid,int offset,int length,char *data,std::string nodeip,int port)
        {
          size_t temp_size;//
          bool reslut=GetFromMemcached(shardid.c_str(),shardid.size(),data,&temp_size,offset,length,nodeip.c_str(),port);
          if(!reslut){
            std::cerr<<"getting from data node fails"<<std::endl;
            return;
          } 
        };

        for(i=0;i<data_idx_ranges.size();i++){
          int idx=data_idx_ranges[i].shardidx;
          int offset=data_idx_ranges[i].offset_in_shard;
          int len=data_idx_ranges[i].range_length;
          std::string shardid=std::to_string(stripeid*1000+idx);
          read_from_datanode(shardid,offset,len,old_shard_data_vector[i].data(),data_nodes_ip_port[i].first,data_nodes_ip_port[i].second);
          std::cout<<"read shard:"<<shardid<<std::endl;
        }

        //calculate datadelta
        
        std::vector<std::thread> delta_calculators;
        
        for(i=0;i<data_idx_ranges.size();i++){
          int len=data_idx_ranges[i].range_length;
          delta_calculators.push_back(std::thread(calculate_data_delta,new_shard_data_vector[i].data(),old_shard_data_vector[i].data(),data_delta_vector[i].data(),len));
          
        }
        for(int i=0;i<delta_calculators.size();i++){
          delta_calculators[i].join();
        }
        

        //update
        if(encode_type==0){//RS
          
          

        }
        else if(encode_type==1){//OPPO LRC 
          
        }
        else{// Azure LRC+1  Best_placement
          asio::ip::tcp::resolver resolver(io_context);
          asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(std::string(collector_proxyip), std::to_string(collector_proxyport));
          asio::ip::tcp::socket socket(io_context);
          asio::connect(socket, endpoint);
          int role=OppoProject::RoleDataProxy;

          std::vector<unsigned char> int_buf_role = OppoProject::int_to_bytes((int)role);
          asio::write(socket, asio::buffer(int_buf_role, int_buf_role.size()));
          //send to collector  
          for(i=0;i<data_idx_ranges.size();i++){
            /*
            std::vector<unsigned char> int_buf_idx=OppoProject::int_to_bytes(data_idx_ranges[i].shardidx);
            asio::write(socket, asio::buffer(int_buf_idx, int_buf_idx.size()));

            std::vector<unsigned char> int_buf_offset=OppoProject::int_to_bytes(data_idx_ranges[i].offset_in_shard);
            asio::write(socket, asio::buffer(int_buf_offset, int_buf_offset.size()));

            std::vector<unsigned char> int_buf_length=OppoProject::int_to_bytes(data_idx_ranges[i].range_length);
            asio::write(socket, asio::buffer(int_buf_length, int_buf_length.size()));
            */
            OppoProject::send_int(socket,data_idx_ranges[i].shardidx);
            OppoProject::send_int(socket,data_idx_ranges[i].offset_in_shard);
            OppoProject::send_int(socket,data_idx_ranges[i].range_length);
            asio::write(socket,asio::buffer(data_delta_vector[i],data_delta_vector[i].size()));
          }

          if(localparity_idxes.size()<1){
            std::cerr<<"where is local parity?? stripeid:"<<stripeid<<std::endl;
          }


           
          for(int i=0;i<localparity_idxes.size();i++){
            std::string local_parity_shardid=std::to_string(stripeid*1000+localparity_idxes[i]);
            for(int j=0;j<data_delta_vector.size();j++)
              DeltaSendToMemcached(local_parity_shardid.c_str(),local_parity_shardid.length(),data_idx_ranges[j].offset_in_shard,data_delta_vector[j].data(),data_delta_vector[j].size(),OppoProject::DeltaType::DataDelta,localparity_nodes_ip_port[i].first.c_str(),localparity_nodes_ip_port[i].second); 
          }

          std::cout<<"Azure LRC+1 data proxy update finished! stripe:"<<stripeid<<std::endl;
          
        };


        //从collector 接受
        

        
        






      }
      catch(const std::exception& e)
      {
        std::cerr << e.what() << '\n';
      }
      return ;
      
    };

    try
    {
      std::thread my_thread(receive_update);
      my_thread.detach();
    }
    catch(const std::exception& e)
    {
      std::cerr << e.what() << '\n';
    }

    return grpc::Status::OK;
  }