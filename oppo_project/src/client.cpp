#include "client.h"
#include "coordinator.grpc.pb.h"
#include "toolbox.h"
#include <asio.hpp>
namespace OppoProject
{
  std::string Client::sayHelloToCoordinatorByGrpc(std::string hello)
  {
    coordinator_proto::RequestToCoordinator request;
    request.set_name(hello);
    coordinator_proto::ReplyFromCoordinator reply;
    grpc::ClientContext context;
    grpc::Status status =
        m_coordinator_ptr->sayHelloToCoordinator(&context, request, &reply);
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
  bool Client::SetParameterByGrpc(ECSchema input_ecschema)
  {
    /*待补充，通过这个函数，要能设置coordinator的编码参数*/
    /*编码参数存储在变量 m_encode_parameter中*/
    coordinator_proto::Parameter parameter;
    parameter.set_partial_decoding((int)input_ecschema.partial_decoding);
    parameter.set_encodetype((int)input_ecschema.encodetype);
    parameter.set_placementtype(input_ecschema.placementtype);
    parameter.set_k_datablock(input_ecschema.k_datablock);
    parameter.set_real_l_localgroup(input_ecschema.real_l_localgroup);
    parameter.set_g_m_globalparityblock(input_ecschema.g_m_globalparityblock);
    parameter.set_b_datapergoup(input_ecschema.b_datapergoup);
    parameter.set_small_file_upper(input_ecschema.small_file_upper);
    parameter.set_blob_size_upper(input_ecschema.blob_size_upper);
    grpc::ClientContext context;
    coordinator_proto::RepIfSetParaSucess reply;
    grpc::Status status = m_coordinator_ptr->setParameter(&context, parameter, &reply);
    if (status.ok())
    {
      return reply.ifsetparameter();
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return false;
    }
  }
  bool Client::set(std::string key, std::string value, std::string flag)
  {

    /* grpc*/
    grpc::ClientContext get_proxy_ip_port;
    coordinator_proto::RequestProxyIPPort request;
    coordinator_proto::ReplyProxyIPPort reply;
    request.set_key(key);
    request.set_valuesizebytes(value.size());
    grpc::Status status = m_coordinator_ptr->uploadOriginKeyValue(
        &get_proxy_ip_port, request, &reply);

    /* grpc*/
    if (!status.ok())
    {
      std::cout << "upload stripe failed!" << std::endl;
      return false;
    }
    else
    {

      std::string proxy_ip = reply.proxyip();
      int proxy_port = reply.proxyport();
      std::cout << "proxy_ip:" << proxy_ip << std::endl;
      std::cout << "proxy_port:" << proxy_port << std::endl;
      asio::io_context io_context;

      asio::error_code error;
      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::resolver::results_type endpoints =
          resolver.resolve(proxy_ip, std::to_string(proxy_port));

      asio::ip::tcp::socket sock_data(io_context);
      asio::connect(sock_data, endpoints);

      std::cout << "key.size()" << key.size() << std::endl;
      std::cout << "value.size()" << value.size() << std::endl;
      std::cout << "proxy_ip:"<<proxy_ip<<std::endl;
      std::cout << "proxy_port:"<<proxy_port<<std::endl;
      asio::write(sock_data, asio::buffer(key, key.size()), error);
      std::cout<<"no error"<<std::endl;
      asio::write(sock_data, asio::buffer(value, value.size()), error);
      std::cout<<"no error!!!!!"<<std::endl;
      asio::error_code ignore_ec;
      sock_data.shutdown(asio::ip::tcp::socket::shutdown_send, ignore_ec);
      sock_data.close(ignore_ec);

      /*这里需要通过检查元数据object_table_big_small_commit来确认是否存成功*/
      grpc::ClientContext check_commit;
      coordinator_proto::AskIfSetSucess request;
      request.set_key(key);
      coordinator_proto::RepIfSetSucess reply;
      grpc::Status status;
      status =
          m_coordinator_ptr->checkCommitAbort(&check_commit, request, &reply);
      std::cout<<"m_coordinator_ptr error!!!!!"<<std::endl;
      if (status.ok())
      {
        if (reply.ifcommit())
        {
          return true;
        }
        else
        {
          std::cout << key << " not commit!!!!!";
        }
      }
      else
      {
        std::cout << key << " Fail to check!!!!!";
      }
    }
    return false;
    /* grpc*/
  }
  bool Client::get(std::string key, std::string &value)
  {
    grpc::ClientContext context;
    coordinator_proto::KeyAndClientIP request;
    request.set_key(key);
    request.set_clientip(m_clientIPForGet);
    request.set_clientport(m_clientPortForGet);
    coordinator_proto::RepIfGetSucess reply;
    grpc::Status status;

    status = m_coordinator_ptr->getValue(&context, request, &reply);

    std::cout<<"get 1"<<std::endl;
    asio::ip::tcp::socket socket_data(io_context);
    int value_size = reply.valuesizebytes();
    
    acceptor.accept(socket_data);
    asio::error_code error;
    std::vector<char> buf_key(key.size());
    std::vector<char> buf(value_size);
    
    size_t len = asio::read(socket_data, asio::buffer(buf_key, key.size()), error);
    std::cout<<"get 2"<<std::endl;
    int flag = 1;
    for (int i = 0; i < int(key.size()); i++)
    {
      if (key[i] != buf_key[i])
      {
        flag = 0;
      }
    }
    std::cout << "value_size:" << value_size << std::endl;
    std::cout << "flag:" << flag << std::endl;
    if (flag)
    {
      len = asio::read(socket_data, asio::buffer(buf, value_size), error);
    }
    asio::error_code ignore_ec;
    socket_data.shutdown(asio::ip::tcp::socket::shutdown_receive, ignore_ec);
    socket_data.close(ignore_ec);
    std::cout << "get key: " << key << " valuesize: " << len << std::endl;
    // for (const auto &c : buf) {
    //   std::cout << c;
    // }
    value = std::string(buf.data(), buf.size());
    std::cout << std::endl;
    return true;
  }
  bool Client::repair(std::vector<int> failed_node_list)
  {
    grpc::ClientContext context;
    coordinator_proto::FailNodes request;
    coordinator_proto::RepIfRepairSucess reply;
    for (int &node : failed_node_list)
    {
      request.add_node_list(node);
    }
    grpc::Status status = m_coordinator_ptr->requestRepair(&context, request, &reply);
    return true;
  }

  bool Client::update_multhread(std::string key, int offset, int length,std::string &new_data,UpdateAlgorithm update_algorithm)
  {
    grpc::ClientContext grpccontext;
    coordinator_proto::UpdatePrepareRequest request;
    coordinator_proto::UpdateDataLocation data_location;
    request.set_key(key);
    request.set_offset(offset);
    request.set_length(length);
    grpc::Status status;
    switch (update_algorithm)
    {
    case RCW:
      status= m_coordinator_ptr->updateRCW(&grpccontext, request, &data_location);
      break;
    case RMW:
      status= m_coordinator_ptr->updateRMW(&grpccontext, request, &data_location);
      break;
    case AZCoordinated:
      status= m_coordinator_ptr->updateGetLocation(&grpccontext, request, &data_location);
      break;
    
    default:
      status= m_coordinator_ptr->updateGetLocation(&grpccontext, request, &data_location);
      break;
    }

    auto update_send_shards=[this](std::string proxy_ip,int proxy_port,int stripeid,int shard_size,
                                  std::vector<ShardidxRange> idx_ranges,
                                  int start_idx,int end_idx,int start_len,bool padding,std::string new_data)
    {
      try
      {

      
      
        asio::ip::tcp::resolver resolver(io_context);
        asio::error_code error;
        //asio::ip::tcp::resolver resolver(io_context2);
        asio::ip::tcp::resolver::results_type endp =
        resolver.resolve(proxy_ip, std::to_string(proxy_port));
        asio::ip::tcp::socket data_socket(io_context);
         asio::connect(data_socket, endp);
                std::cout<<"connect"<<proxy_ip<<" : "<<proxy_port<<"   "<<error.message()<<std::endl;

        OppoProject::send_int(data_socket,(int) OppoProject::RoleClient);
        std::cout<<"stripe id"<<std::endl;
        OppoProject::send_int(data_socket,stripeid);
        int count=idx_ranges.size();
        OppoProject::send_int(data_socket,count);
        for(int t=0;t<count;t++){
          int idx=idx_ranges[t].shardidx;
          int offset_in_shard=idx_ranges[t].offset_in_shard;
          int length_in_shard=idx_ranges[t].range_length;
          std::cout<<"length_in_shard: "<<length_in_shard<<std::endl;

          OppoProject::send_int(data_socket,idx);
          if(padding)
          {
            OppoProject::send_int(data_socket,0);
            OppoProject::send_int(data_socket,shard_size);
          }
          else{
            OppoProject::send_int(data_socket,offset_in_shard);
            OppoProject::send_int(data_socket,length_in_shard);
          }



          std::cout<<"idx: "<<idx<<std::endl;
          std::string data_in_shard;//to send
          if(idx==start_idx    && padding && offset_in_shard!=0)
          {
            int zero_len=offset_in_shard;
            data_in_shard=std::string(shard_size,'0');
            bzero((void*) data_in_shard.data(),zero_len);
            data_in_shard.replace(data_in_shard.begin()+offset_in_shard,data_in_shard.end(),
                                  new_data.substr(0,length_in_shard));
          }
          else if(idx==end_idx && padding && length_in_shard!=shard_size )
          {
            int zero_len=shard_size-length_in_shard;
            data_in_shard=std::string(shard_size,'0');
            bzero((void*)data_in_shard.data()+length_in_shard,zero_len);
            data_in_shard.replace(data_in_shard.begin(),data_in_shard.begin()+length_in_shard,
                                  new_data.substr(start_len+(idx-start_idx-1)*(shard_size),length_in_shard));

          }
          else if(idx==start_idx && !(padding && offset_in_shard!=0))
            data_in_shard=new_data.substr(0,length_in_shard);
          else data_in_shard=new_data.substr(start_len+(idx-start_idx-1)*(shard_size),length_in_shard);




          asio::write(data_socket,asio::buffer(data_in_shard,data_in_shard.size()),error);

          std::cout<<" idx:"<<idx<<" data offset: "<<offset_in_shard<<" length: "<<length_in_shard<<std::endl;
          std::cout<<" data is:"<<data_in_shard<<std::endl;
        }

        data_socket.shutdown(asio::ip::tcp::socket::shutdown_send, error);
        data_socket.close(error);
      }
      catch(const std::exception& e)
      {
        std::cerr << e.what() << '\n';
      }

    };

    if(status.ok())
    {
      try
      {
        if (key != data_location.key())
            std::cerr << "what!!! coordinator returns other object's update soluuution!!" << std::endl;
      
        
        std::vector<std::pair<std::string, int>> proxy_ipports;
        std::vector<std::vector<OppoProject::ShardidxRange>> ranges_of_proxys;
        int i = 0;
        int j = 0;//tanverse all idx 
        for(i=0;i<data_location.proxyip_size();i++)
        {
          std::string proxy_ip = data_location.proxyip(i);
          int proxy_port = data_location.proxyport(i);
          proxy_ipports.push_back(std::pair<std::string, int>(proxy_ip,proxy_port));
          int count = data_location.num_each_proxy(i);
          std::vector<OppoProject::ShardidxRange> ranges;
          for(int t=0;t<count;t++){
            
            int idx=data_location.datashardidx(j);
            int offset_in_shard=data_location.offsetinshard(j);
            int length_in_shard=data_location.lengthinshard(j);
            ranges.push_back(ShardidxRange(idx,offset_in_shard,length_in_shard));
            j++;//
          }
          ranges_of_proxys.push_back(ranges);
        }
        int start_idx=data_location.datashardidx(0);
        int end_idx=data_location.datashardidx(0);
        int start_len=0;
        int updated_len=0;//记录本次更新的长度
        for(int tttt=0;tttt<data_location.datashardidx_size();tttt++)
        {
          if(data_location.datashardidx(tttt)<start_idx)
          {
            start_idx=data_location.datashardidx(tttt);
            start_len=data_location.lengthinshard(tttt);
          } 
          if(data_location.datashardidx(tttt)>end_idx) end_idx=data_location.datashardidx(tttt);
          updated_len+=data_location.lengthinshard(tttt);
        }

        std::vector<std::thread> senders;
        for(int i=0;i<proxy_ipports.size();i++)
        {
          senders.push_back(std::thread(update_send_shards,proxy_ipports[i].first,proxy_ipports[i].second,data_location.stripeid(),data_location.shardsize(),
                                                          ranges_of_proxys[i],start_idx,end_idx,start_len,data_location.padding_to_shard(),new_data));
        }

        for(int i=0;i<senders.size();i++)
          senders[i].join();


        grpc::ClientContext check_commit;
        coordinator_proto::AskIfSetSucess request;
        request.set_key(key);
        coordinator_proto::RepIfSetSucess reply;
        grpc::Status status;
        status =
            m_coordinator_ptr->checkUpdateFinished(&check_commit, request, &reply);
        std::cout<<"m_coordinator_ptr error!!!!!"<<std::endl;
        if (status.ok())
        {
          if (reply.ifcommit())
          {
            return true;
          }
          else
          {
            std::cout << key << " update fail!!!!!";
          }
        }
        else
        {
          std::cout << key << " Fail to check!!!!!";
        }
        
      }
      catch(const std::exception& e)
      {
        std::cerr << e.what() << '\n';
      }
      
    }
  }

  bool Client::update(std::string key, int offset, int length,std::string &new_data,UpdateAlgorithm update_algorithm)
  {
    grpc::ClientContext grpccontext;
    coordinator_proto::UpdatePrepareRequest request;
    coordinator_proto::UpdateDataLocation data_location;
    request.set_key(key);
    request.set_offset(offset);
    request.set_length(length);
    grpc::Status status;
    switch (update_algorithm)
    {
    case RCW:
      status= m_coordinator_ptr->updateRCW(&grpccontext, request, &data_location);
      break;
    case RMW:
      status= m_coordinator_ptr->updateRMW(&grpccontext, request, &data_location);
      break;
    case AZCoordinated:
      status= m_coordinator_ptr->updateGetLocation(&grpccontext, request, &data_location);
      break;
    
    default:
      status= m_coordinator_ptr->updateGetLocation(&grpccontext, request, &data_location);
      break;
    }
     
    if (status.ok())
    {
      std::cout<<"get location success"<<std::endl;
      if(data_location.update_buffer())
      {
        std::string proxy_ip = data_location.proxyip(0);
        int proxy_port = data_location.proxyport(0);
        std::cout << "proxy_ip:" << proxy_ip << std::endl;
        std::cout << "proxy_port:" << proxy_port << std::endl;
        asio::io_context io_context;

        asio::error_code error;
        asio::ip::tcp::resolver resolver(io_context);
        asio::ip::tcp::resolver::results_type endpoints =
            resolver.resolve(proxy_ip, std::to_string(proxy_port));

        asio::ip::tcp::socket sock_data(io_context);
        std::cout<<"try to connect "<<proxy_ip<<" "<<proxy_port<<std::endl;
        asio::connect(sock_data, endpoints);

        std::cout << "key.size()" << key.size() << std::endl;
        std::cout << "value.size()" << new_data.size() << std::endl;
        std::cout << "proxy_ip:"<<proxy_ip<<std::endl;
        std::cout << "proxy_port:"<<proxy_port<<std::endl;
        asio::write(sock_data, asio::buffer(key, key.size()), error);
        std::cout<<"no error"<<std::endl;
        asio::write(sock_data, asio::buffer(new_data, new_data.size()), error);
        std::cout<<"no error!!!!!"<<std::endl;
        asio::error_code ignore_ec;
        sock_data.shutdown(asio::ip::tcp::socket::shutdown_send, ignore_ec);
        sock_data.close(ignore_ec);
      }
      else{
      
        std::cout<<"stripeid:"<<data_location.stripeid()<<std::endl;
        try
        {
          asio::error_code error;
          if (key != data_location.key())
            std::cerr << "what!!! coordinator returns other object's update soluuution!!" << std::endl;
          int descending_len = length;
          int rest_len=length;
          int send_start_position=0;

          std::vector<std::pair<std::string, int>> proxy_ipports;
          std::vector<std::vector<OppoProject::ShardidxRange>> ranges_of_proxys;
          int i = 0;
          int j = 0;//tanverse all idx 

          asio::io_context io_context2;

          int start_idx=data_location.datashardidx(0);
          int end_idx=data_location.datashardidx(0);
          int start_len=0;
          for(int tttt=0;tttt<data_location.datashardidx_size();tttt++)
          {
            if(data_location.datashardidx(tttt)<start_idx)
            {
              start_idx=data_location.datashardidx(tttt);
              start_len=data_location.lengthinshard(tttt);
            } 
            if(data_location.datashardidx(tttt)>end_idx) end_idx=data_location.datashardidx(tttt);
          }

          std::cout<<"start_idx: "<<start_idx<<std::endl;

          asio::ip::tcp::resolver resolver2(io_context2);
          std::string send_data(new_data.length(),'0');
          for (i = 0; i < data_location.proxyip_size(); i++)
          {
            std::string proxy_ip = data_location.proxyip(i);

            int proxy_port = data_location.proxyport(i);
            int count = data_location.num_each_proxy(i);
            asio::error_code error;
            //asio::ip::tcp::resolver resolver(io_context2);
            asio::ip::tcp::resolver::results_type endp =
                resolver2.resolve(proxy_ip, std::to_string(proxy_port));

            asio::ip::tcp::socket data_socket(io_context2);
            std::cout<<"try to connect "<<proxy_ip<<" "<<proxy_port<<std::endl;
            asio::connect(data_socket, endp);
            std::cout<<"connect"<<proxy_ip<<" : "<<proxy_port<<"   "<<error.message()<<std::endl;

            OppoProject::send_int(data_socket,(int) OppoProject::RoleClient);
            std::cout<<"stripe id"<<std::endl;
            OppoProject::send_int(data_socket,data_location.stripeid());
            OppoProject::send_int(data_socket,count);

            for(int t=0;t<count;t++){
              int idx=data_location.datashardidx(j);
              int offset_in_shard=data_location.offsetinshard(j);
              int length_in_shard=data_location.lengthinshard(j);
              std::cout<<"length_in_shard: "<<length_in_shard<<std::endl;
              j++;//!!
              OppoProject::send_int(data_socket,idx);
              OppoProject::send_int(data_socket,offset_in_shard);
              OppoProject::send_int(data_socket,length_in_shard);
              std::cout<<"idx: "<<idx<<std::endl;
              std::string data_in_shard=new_data.substr((idx-start_idx)*(length_in_shard),length_in_shard);
              asio::write(data_socket,asio::buffer(data_in_shard,data_in_shard.size()),error);
              send_data.replace(send_data.begin()+(idx-start_idx)*(length_in_shard),send_data.begin()+(idx-start_idx)*(length_in_shard)+length_in_shard,data_in_shard);
              std::cout<<" idx:"<<idx<<" data offset: "<<offset_in_shard<<" length: "<<length_in_shard<<std::endl;
              std::cout<<" data is:"<<data_in_shard<<std::endl;
              rest_len-=length_in_shard;
              send_start_position+=length_in_shard; 
            }

            data_socket.shutdown(asio::ip::tcp::socket::shutdown_send, error);
            data_socket.close(error);

          }

          if(send_data!=new_data) std::cout<<"send update data error!"<<std::endl;
          else std::cout<<"send update data success!"<<std::endl;

        }
        catch(const std::exception& e)
        {
          std::cerr << e.what() << '\n';
        }
      }

      std::cout<<"waiting for proxy update success"<<std::endl;

      grpc::ClientContext check_commit;
      coordinator_proto::AskIfSetSucess request;
      request.set_key(key);
      coordinator_proto::RepIfSetSucess reply;
      grpc::Status status;
      status =
          m_coordinator_ptr->checkUpdateFinished(&check_commit, request, &reply);
      std::cout<<"m_coordinator_ptr error!!!!!"<<std::endl;
      if (status.ok())
      {
        if (reply.ifcommit())
        {
          return true;
        }
        else
        {
          std::cout << key << " update fail!!!!!";
        }
      }
      else
      {
        std::cout << key << " Fail to check!!!!!";
      }
      
    }
    else{
      return false;
      std::cerr<<"get update location failed"<<std::endl;
    }

    
    

    return true;
  }

  



  bool Client::write_to_logmanager(const char *key,size_t key_length,int offset_in_shard,const char *update_data,size_t update_data_length,int delta_type,const char* ip,int port)
  {
    try{
      std::cout << "DeltaSendToMemcached"
                << " " << std::string(ip) << " " << port << std::endl;
      std::cout << "key:"<<std::string(key)<<std::endl;
      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(std::string(ip), std::to_string(port));
      asio::ip::tcp::socket socket(io_context);
      asio::connect(socket, endpoint);
      std::cout<<"connect success \n";
      int flag = 2;
      OppoProject::send_int(socket,flag);
      OppoProject::send_int(socket,key_length);
      OppoProject::send_int(socket,update_data_length);

      
      asio::write(socket, asio::buffer(key, key_length));
      asio::write(socket, asio::buffer(update_data, update_data_length));
      
      std::cout<<"send data is "<<update_data<<std::endl;

      OppoProject::send_int(socket,offset_in_shard);
      OppoProject::send_int(socket,delta_type);


      

      std::vector<char> finish(1);
      asio::read(socket, asio::buffer(finish, finish.size()));

      asio::error_code ignore_ec;
      socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
      socket.close(ignore_ec);

    }
    catch (std::exception &e)
    {
      std::cout << e.what() << std::endl;
    }

    // std::cout << "set " << std::string(key) << " " << std::string(ip) << " " << port << " " << (MEMCACHED_SUCCESS==rc) << " " << value_length << std::endl;
    return true;
  }
} // namespace OppoProject