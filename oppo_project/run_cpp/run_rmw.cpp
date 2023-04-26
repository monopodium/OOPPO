#include "meta_definition.h"
#include "azure_lrc.h"
#include "toolbox.h"
#include "client.h"
#include <fstream>
bool RMW_test(std::string olddata,std::string newdata,int offset,int length,int blocksize,std::vector<int> data_idxes,std::vector<int> parity_idxes,
              int k,int m,int real_l,OppoProject::EncodeType encode_type);

int run_rmw();
void split_to_data_blocks(std::string &string,std::vector<char *> &v_data,char ** data,int k,int blocksize);



int main(int argc, char **argv)
{

  if (argc != 11 && argc != 12)
  {
    std::cout<<argc<<std::endl;
    std::cout << "./run_client partial_decoding encode_type placement_type k real_l g small_file_upper blob_size_upper trace value_length" << std::endl;
    std::cout << "./run_client false RS Random 3 -1 2 1024 4096 random 2048" << std::endl;
    exit(-1);
  }

  bool partial_decoding;
  OppoProject::EncodeType encode_type;
  OppoProject::PlacementType placement_type;
  int k, real_l, g_m, b;
  int small_file_upper, blob_size_upper;
  int value_length;

  partial_decoding = (std::string(argv[1]) == "true");
  if (std::string(argv[2]) == "OPPO_LRC")
  {
    encode_type = OppoProject::OPPO_LRC;
  }
  else if (std::string(argv[2]) == "Azure_LRC_1")
  {
    encode_type = OppoProject::Azure_LRC_1;
  }
  else if (std::string(argv[2]) == "RS")
  {
    encode_type = OppoProject::RS;
  }
  else
  {
    std::cout << "error: unknown encode_type" << std::endl;
    exit(-1);
  }
  if (std::string(argv[3]) == "Flat")
  {
    placement_type = OppoProject::Flat;
  }
  else if (std::string(argv[3]) == "Random")
  {
    placement_type = OppoProject::Random;
  }
  else if (std::string(argv[3]) == "Best_Placement")
  {
    placement_type = OppoProject::Best_Placement;
  }
  else
  {
    std::cout << "error: unknown placement_type" << std::endl;
    exit(-1);
  }
  k = std::stoi(std::string(argv[4]));
  real_l = std::stoi(std::string(argv[5]));
  b = std::ceil((double)k / (double)real_l);
  g_m = std::stoi(std::string(argv[6]));
  small_file_upper = std::stoi(std::string(argv[7]));
  blob_size_upper = std::stoi(std::string(argv[8]));
  value_length = std::stoi(std::string(argv[10]));
  std::string client_ip, coordinator_ip;
  if (argc == 12) {
    client_ip = std::string(argv[11]);
  } else {
    client_ip = "0.0.0.0";
  }
  coordinator_ip = client_ip;

  OppoProject::Client client(client_ip, 44444, coordinator_ip + std::string(":55555"));
  /**测试**/
  std::cout << client.sayHelloToCoordinatorByGrpc("MMMMMMMM") << std::endl;
  /**测试**/

  /**设置编码参数的函数，咱就是说有用没用都给传过去存下来，
   * 现在的想法就是每次需要修改这个参数，都要调用一次这个函数来改**/

  if (client.SetParameterByGrpc({partial_decoding, encode_type, placement_type, k, real_l, g_m, b, small_file_upper, blob_size_upper}))
  {
    std::cout << "set parameter successfully!" << std::endl;
  }
  else
  {
    std::cout << "Failed to set parameter!" << std::endl;
  }

  std::unordered_map<std::string, std::string> key_values;
  /*生成随机的key value对*/

  if (std::string(argv[9]) == "random")
  {
    
    
    for (int i = 0; i < 100; i++)

    {
      std::string key;
      std::string value;
      //OppoProject::random_generate_kv(key, value, 6, value_length);
      OppoProject::random_generate_value(key,6);
      while (key_values.find(key)!=key_values.end()) OppoProject::random_generate_value(key,6);
      OppoProject::random_generate_value(value,value_length);
      
      key_values[key] = value;
      std::cout << key.size() << std::endl;
      std::cout << key << std::endl;
      std::cout << value.size() << std::endl;

      client.set(key, value, "00");

      std::cout<<"client.set(key, value,)"<<std::endl;

      std::string get_value;
      client.get(key, get_value);
      
      if (value == get_value) {
        std::cout << "set kv successfully" << std::endl;
      } else {
        std::cout << "wrong!" << std::endl;
        break;
      }
    }
    
    
  }
  else if (std::string(argv[9]) == "ycsb")
  {
    std::string line;
    char inst;
    std::string key;
    std::string warm_path = "../../third_party/YCSB-tracegen/warm.txt";
    std::string test_path = "../../third_party/YCSB-tracegen/test.txt";
    std::ifstream inf;
    for (int p = 0; p < 2; p++)
    {
      if (p == 0)
        inf.open(warm_path);
      else
        inf.open(test_path);
      if (!inf)
        std::cerr << "cannot open the file" << std::endl;
      while (getline(inf, line))
      {
        const char *delim = " ";
        inst = line[0];
        int pos = line.find(delim, 2);
        key = line.substr(2, pos - 2);
        if (inst == 'I')
        {
          std::string value;
          OppoProject::random_generate_value(value, value_length);
          key_values[key] = value;
          std::cout << key.size() << std::endl;
          std::cout << key << std::endl;
          std::cout << value.size() << std::endl;
          client.set(key, value, "00");
        }
        else if (inst == 'R')
        {
          std::string get_value;
          std::cout << key.size() << std::endl;
          std::cout << "run_client get (key): " << key << std::endl;
          client.get(key, get_value);
          std::cout << "run_client finish get (key): " << key << std::endl;
          std::cout << get_value.size() << std::endl;
          if (key_values[key] == get_value)
          {
            std::cout << "set kv successfully" << std::endl;
          }
          else
          {
            std::cout << "*********************wrong!********************" << std::endl;
            break;
          }
        }
      }
      inf.close();
    }
  }
  std::cout << "set/get finish!" << std::endl;


  
  

  

  //update test
  

  
  //std::string new_v1="AgFZJOeRSukFbkinscTxiQmrvKUkEswXziztHDMWitsSQuGjkmCZoTjjXAUGRxalZIWqcgVtTJtAyfUZlxcDZAobSNwlYXAnBaCErzWrJRcWsZEneCvjeJErhnygrwuMDTggKsvHClRdIEPeqOnWagqboEAARBNKCfnaAskbwGQyZdcHVlwszQcOHExregOlIMBlJEWiRFONGBLJjRbBSTMkhJapebTyzVhjgEEnrHNZvyriLlDeNZBqtqMJrgRHKLBvJJUJTExfFQZxDtQUSrTcXBpUijUlIfgRFQxqZBfVkpQkrVywSJHABFLwKrhXZCzkCuNEttUkPsjclQWGqrZjBDsbEdPTIfVdOVNkyDUmIYVgCCaSnMiunfDPdSjWErmSqySJJBccOqTWJGnPXMcCUfdizKyOJRWIJyhHFwxqsZQDyDfpjkBXsLdhPgDdvmxhYRfTnzQyeWPklzGIzKEGiDKUeaUtjtvKkSAkILTkCRRzPglhFanhqqvSCglFjzDPXNthBfTZiafVIiGZcLrMwvqEVxdeHQFYbxgPFVBoTIAolLjZzyhdmTTRtpktnzmQqnzCAkipRVSnIpkVpzGUtPQaZNlSzcnqwLFYxRSTtAuYMkBzISfXijtZUpNcBsvNloMLPqlOBVjdtVzPVKLjSqwIDqpxiqxTGZqlIuRabSONxYVhnPpKDATGJJfCALpmDCxhzbmLRTjoXfammpWHOetolqmCuGqPecuDawsWDLPapoFLKwJwCUcyFzmwpJVJGOgfeSYqlwExMaJDkCDsDKuilLfhdTQWcmaRbVNncftpITjzgXXczXmBOFQdyjXAUAezBdnQsNsGrlcArGjWxcuHBEsZQBdOEzrMXqCowHpEIIRSKRyscCJBCaZjjdmuNosrSJXXsEuwYCUqfJsdYRcVkAzaZzYTeStKcQWdBywKfiShziuUAMlqPIvsJMUTpILWRbpTixvYlRNeLHEkMQiuOUKWaRMhvEHwNVkAPvWpNOKnGULCDEmyMRXGeEZwagUliXBecHmNNuIyTgxdgRfVAtzQ";
  



  
  
  
  for(auto const &temp_kv:key_values)
  {
    
    auto k1=temp_kv.first;
    auto v1=temp_kv.second;
    std::string new_v1;
    OppoProject::random_generate_value(new_v1,2048/2+32);
    int update_len=new_v1.length();
    std::cout<<"key1:"<<k1<<std::endl;
    std::cout<<"v1.len"<<v1.length();
    std::cout<<"v1 before update "<<std::endl;
    std::cout<<temp_kv.second;

    //OppoProject::random_generate_value(new_v1,update_len);
    //OppoProject::random_generate_value(new_v1,176);
    std::cout<<"update len: "<<new_v1.length()<<std::endl;
    client.update(k1,0,new_v1.length(),new_v1);
    key_values[k1].replace(0,update_len,new_v1);
    std::cout<<"v1 after update "<<std::endl;
    std::cout<<key_values[k1];
    std::cout<<std::endl;
  }

  
  
  
  
  
 
  std::cout << "开始修复" << std::endl;
  // 这里其实应该用ip
  // 但目前我们是在单机上进行测试的，所以暂时用端口号代替一下

  std::cout << "开始修复" << std::endl;
  //这里其实应该用ip
  //但目前我们是在单机上进行测试的，所以暂时用端口号代替一下
  
  for (int j = 0; j < 60; j++)

  {
    int temp = j;
    std::cout << "repair" << temp << std::endl;
    std::vector<int> failed_node_list = {temp};
    client.repair(failed_node_list);
  }
  
  for (int i = 0; i < 10; i++)
  {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned int> dis(0, 59);
    std::unordered_set<int> help;
    std::vector<int> failed_node_list;
    int p;
    for (int i = 0; i < 6; i++) {
      do {
        p = dis(gen);
      } while(help.count(p) > 0);
      help.insert(p);
      failed_node_list.push_back(p);
    }
    client.repair(failed_node_list);
  }
  

  

  

  /**/
  int success=0;
  int failed=0;
  for(auto const &temp_kv:key_values)
  {
    
    std::string temp;
    client.get(temp_kv.first,temp);
    std::cout<<"updated value:"<<std::endl;
    std::cout<<temp_kv.second<<std::endl;
    std::cout<<"get value:"<<std::endl;
    std::cout<<temp<<std::endl;
    
    if(temp!=temp_kv.second)
    {
      std::cout<<"G!!!!!!!"<<std::endl;
      failed++;
    }
    
    else 
    {
      std::cout<<"6666!!!!!!!!"<<std::endl;
      success++;
    }
    

  }
  std::cout<<"sucess:"<<success<<" failed: "<<failed<<std::endl;
  
  


  //RMW_test(local_v,new_v1,0,new_v1.length(),176,dataidxes,parityidxes,12,6,0,OppoProject::RS);

  
  
  
}
/*
{
    std::string test_k;
  OppoProject::random_generate_value(test_k,6);
  
  while(key_values.find(test_k)!=key_values.end()) OppoProject::random_generate_value(test_k,6);
  std::cout<<"file key "<<test_k<<std::endl;
  std::string test_v;
  OppoProject::random_generate_value(test_v,2048);
  
  std::string local_v=test_v;
  std::cout<<"v1 before update:"<<std::endl;
  std::cout<<test_v<<std::endl;
  key_values[test_k]=test_v;


  client.update(test_k,0,new_v1.length(),new_v1);
  key_values[test_k].replace(0,update_len,new_v1);
  std::cout<<"get location sucess"<<std::endl;
  std::cout<<"v1 after update "<<std::endl;
  std::cout<<key_values[test_k];
  std::cout<<std::endl;

  
  std::vector<int> dataidxes;
  std::vector<int> parityidxes;
  for(int i=0;i<6;i++) dataidxes.push_back(i);
  for(int i=12;i<12+6;i++) parityidxes.push_back(i);
  std::cout<<" local v before update: "<<std::endl;
  
  std::cout<<"new_v1:"<<std::endl;
  std::cout<<new_v1<<std::endl;
  std::cout<<"after update"<<std::endl;
  std::cout<<key_values.begin()->second<<std::endl;


  std::cout<<local_v<<std::endl;
}
*/
void split_to_data_blocks(std::string &string,std::vector<char *> &v_data,char ** data,int k,int blocksize)
{
    char* buf=const_cast<char*> (string.data());
    for (int j = 0; j < k; j++)
    {
      data[j] = &buf[j * blocksize];
    }
}


int run_rmw()
{
    for(int ccc=0;ccc<100;ccc++)
    {
        std::string value;
        std::string key;
        OppoProject::random_generate_kv(key, value, 6, 2048);
        std::vector<int> data_idxes;
        std::vector<int> parity_idxes;
        int blocksize=176;
        int k=12;
        int m=6;

        std::string value2;
        OppoProject::random_generate_value(value2,1024+32);
        std::cout<<"value2 len:<<"<<value2.length()<<std::endl;
        std::cout<<value2<<std::endl<<std::endl;


        int start_block=0;

        int u_block_num=value2.size()/blocksize;


        std::cout<<"pos in old"<<value2.length()<<std::endl;
        std::cout<<value.substr(start_block*blocksize,value2.length())<<std::endl<<std::endl;


        for(int i=0;i<u_block_num;i++)
            data_idxes.push_back(start_block+i);

        for(int j=0;j<m;j++)
            parity_idxes.push_back(j+k);

        RMW_test(value,value2,start_block*blocksize,value2.length(),blocksize,data_idxes,parity_idxes,k,m,0,OppoProject::RS);
        
    }
    return 0;
}

bool RMW_test(std::string olddata,std::string newdata,int offset,int length,int blocksize,std::vector<int> data_idxes,std::vector<int> parity_idxes,
              int k,int m,int real_l,OppoProject::EncodeType encode_type)
{  
  int sub_k=data_idxes.size();
  int sub_m=parity_idxes.size();
  int tali_size=k*blocksize-olddata.length();

  std::cout<<"tail:"<<tali_size<<std::endl;


  std::vector<char *> v_data(k);
  std::vector<char *> v_coding(m + real_l + 1);
  char **data = (char **)v_data.data();
  char **coding = (char **)v_coding.data();

  int true_shard_size = blocksize;
  std::vector<std::vector<char>> data_area(k, std::vector<char>(true_shard_size));
  std::vector<std::vector<char>> v_coding_area(m + real_l + 1, std::vector<char>(true_shard_size));


  auto set_ptrs=[](std::vector<char*> &ptrs,std::vector<std::vector<char>> &vec)
  {
    int t=vec.size();
    for(int i=0;i<t;i++)
        ptrs[i]=vec[i].data();
  };

  set_ptrs(v_data,data_area);


  for (int j = 0; j < k-1; j++)
  {
    memcpy(data_area[j].data(),olddata.c_str()+j*true_shard_size,true_shard_size);
  }
  memcpy(data_area[k-1].data(),olddata.c_str()+(k-1)*true_shard_size,true_shard_size-tali_size);



  for(int i=1;i<=tali_size;i++)
    data[k-1][blocksize-i]='0';

  std::cout<<"old:"<<std::endl<<olddata<<std::endl;
  for(int i=0;i<k;i++) std::cout<<std::string(data[i],true_shard_size);
  std::cout<<'\n';

  for (int j = 0; j < m + real_l + 1; j++)
  {
    coding[j] = v_coding_area[j].data();
  }

  if (encode_type == OppoProject::RS)
  {
    encode(k, m, 0, data, coding, true_shard_size, encode_type);

  }
  else if (encode_type == OppoProject::Azure_LRC_1)
  {
    // m = g for lrc
    encode(k, m, real_l, data, coding, true_shard_size, encode_type);

  }
  else if (encode_type == OppoProject::OPPO_LRC)
  {

    encode(k, m, real_l, data, coding, true_shard_size, encode_type);

  }




  auto repair_test=[=](char **r_data,char **r_coding,std::string origin)
  {
    // for(int i=0;i<m;i++)
    // {
    //   std::cout<<"parity "<<i<<std::endl;
    //   std::cout<<r_coding[i]<<std::endl;
    // }

    

    int expect_block_number = (encode_type ==OppoProject::Azure_LRC_1) ? (k + real_l - 1) : k;
    int all_expect_blocks = (encode_type ==OppoProject::Azure_LRC_1) ? (k + m + real_l) : (k + m);
    auto erasures = std::make_shared<std::vector<int>>();
    
    /*rand fail*/
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned int> dis(0, k+m+real_l-1);

    int random_fail = dis(gen);
    std::cout<<"failed block: "<<random_fail<<std::endl;

    erasures->push_back(2);
    erasures->push_back(5);

    erasures->push_back(0);
    erasures->push_back(1);
    erasures->push_back(-1);

    bzero(r_data[2],true_shard_size);
    bzero(r_data[5],true_shard_size);

    std::string value;

    if (encode_type == OppoProject::RS || encode_type == OppoProject::OPPO_LRC)
    {
      if(!decode(k, m, 0, r_data, r_coding, erasures, true_shard_size, encode_type)) std::cout<<"decode wrong"<<std::endl;
    }
    else if (encode_type == OppoProject::Azure_LRC_1)
    {
      if (!decode(k, m, real_l, r_data, r_coding, erasures, true_shard_size, encode_type))
      {
        std::cout << "cannot decode!" << std::endl;
      }
    }

    
    
      
    
    for (int j = 0; j < k-1; j++)
    {
      value += std::string(r_data[j], true_shard_size);
    }
    value+=std::string(r_data[k-1], true_shard_size-tali_size);
    std::cout<<"repaired:"<<std::endl<<value<<std::endl;
    std::cout<<std::endl;
    if(value==origin)
    {
        std::cout<<"repair success"<<std::endl;
        std::cout<<"origin"<<std::endl;
        std::cout<<origin<<std::endl;
        return true;
    }
    else
    {
        std::cout<<"repair fail"<<std::endl;

        std::cout<<"origin"<<std::endl;
        std::cout<<origin<<std::endl;
        std::cout<<std::endl<<std::endl<<std::endl;

        std::cout<<"repaied"<<std::endl;
        std::cout<<value<<std::endl;
        return false;
    }



  };

  //repair_test(data,coding,olddata);

  
  
  std::vector<std::vector<char>> data_deltas(data_idxes.size(),std::vector<char>(true_shard_size));
  std::vector<char *> data_delta_ptrs(data_idxes.size());
  set_ptrs(data_delta_ptrs,data_deltas);

  std::vector<std::vector<char>> parity_deltas(m,std::vector<char>(true_shard_size));
  std::vector<char*> g_parity_delta_ptrs(m);
  set_ptrs(g_parity_delta_ptrs,parity_deltas);

  /*打乱 global idx顺序*/
  std::vector<int> unordered_parity_idxes=parity_idxes;
  std::vector<char*> unordered_g_parity_delta_ptrs(m);
  std::vector<int> unordered_data_idxes=data_idxes;
  std::vector<char*> unordered_data_delta_ptrs(k);
  std::random_device rd;
  std::mt19937 rand_gen(rd());
  std::shuffle(unordered_parity_idxes.begin(),unordered_parity_idxes.end(),rand_gen);
  for(int i=0;i<unordered_parity_idxes.size();i++)
  {
    int big_idx=unordered_parity_idxes[i];
    int small_idx=unordered_parity_idxes[i]-k;
    std::cout<<"i: "<<small_idx<<" i+k "<<big_idx<<std::endl;
    std::cout<<"gdelta addr:"<<g_parity_delta_ptrs[small_idx]<<std::endl;
    unordered_g_parity_delta_ptrs[i]=g_parity_delta_ptrs[small_idx];
  }

  std::shuffle(unordered_data_idxes.begin(),unordered_data_idxes.end(),rand_gen);
  for(int i=0;i<unordered_data_idxes.size();i++)
  {
    std::cout<<"i: "<<i<<" to  "<<unordered_data_idxes[i]<<std::endl;
    unordered_data_delta_ptrs[i]=data_delta_ptrs[unordered_data_idxes[i]];
  }

  

  


  std::vector<std::vector<char>> v_new_data(data_idxes.size(),std::vector<char>(true_shard_size));
  std::vector<char *> newdata_ptrs(data_idxes.size());
  set_ptrs(newdata_ptrs,v_new_data);

  
  for(int i=0;i<data_idxes.size();i++)
  {
    memcpy(newdata_ptrs[i],newdata.c_str()+i*true_shard_size,true_shard_size);
  }

  for(int i=0;i<data_idxes.size();i++)
  {
    std::cout<<"new data block "<<i<<std::endl;
    std::cout<<std::string(newdata_ptrs[i],true_shard_size)<<std::endl;
  }

  auto print_vec=[blocksize](std::vector<int> &idxes,std::vector<char*> &vec)
  {
    for(int i=0;i<idxes.size();i++)
    {
        std::cout<<"idx: "<<idxes[i]<<std::endl;
        std::cout<<std::string(vec[i],blocksize)<<std::endl;
    }
  };



  for(int i=0;i<data_idxes.size();i++)
  {

    std::cout<<"calculate delta  "<<i<<std::endl;
    OppoProject::calculate_data_delta(newdata_ptrs[i],data[data_idxes[i]],data_delta_ptrs[i],true_shard_size);
  }

  //std::cout<<"data delta \n";
  //print_vec(data_idxes,data_delta_ptrs);

  OppoProject::calculate_global_parity_delta(k,m,real_l,unordered_data_delta_ptrs.data(),unordered_g_parity_delta_ptrs.data(),true_shard_size,unordered_data_idxes,unordered_parity_idxes,encode_type);
  //std::cout<<"parity delta \n";
  //print_vec(unordered_parity_idxes,unordered_g_parity_delta_ptrs);
  
  std::vector<std::vector<char>> v_new_parity(parity_idxes.size(),std::vector<char>(true_shard_size));
  std::vector<char *> new_parity_ptrs(parity_idxes.size());
  set_ptrs(new_parity_ptrs,v_new_parity);

  //data inplace update
  for(int i=0;i<data_idxes.size();i++)
  {
    //OppoProject::calculate_data_delta(data[data_idxes[i]],data_delta_ptrs[i],data[data_idxes[i]],true_shard_size);
    memcpy(data[data_idxes[i]],newdata_ptrs[i],true_shard_size);
  }

  
  //parity update
  for(int j=0;j<parity_idxes.size();j++)
  {
    OppoProject::calculate_data_delta(g_parity_delta_ptrs[j],coding[j],new_parity_ptrs[j],true_shard_size);
  }
  std::cout<<"new parity \n";
  //print_vec(parity_idxes,new_parity_ptrs);
  auto combine_to_string=[true_shard_size,tali_size,k](char ** data)
  {
    std::string temp;
    for(int i=0;i<k-1;i++)
      temp+=std::string(data[i],true_shard_size);
    temp+=std::string(data[k-1],true_shard_size-tali_size);
    return temp;
  };
  std::string str=combine_to_string(data);
  std::cout<<"data after update"<<std::endl<<str<<std::endl;

  
  repair_test(data,new_parity_ptrs.data(),str);
  return 0;
  
  


  
    

}

