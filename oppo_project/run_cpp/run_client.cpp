#include "client.h"
#include "toolbox.h"

#include <fstream>

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
    /*
    
    for (int i = 0; i < 100; i++)

    {
      std::string key;
      std::string value;
      OppoProject::random_generate_kv(key, value, 6, value_length);
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
    */
    
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
  
  std::string test_k;
  OppoProject::random_generate_value(test_k,6);
  while(key_values.find(test_k)!=key_values.end()) OppoProject::random_generate_value(test_k,6);
  std::string test_v="ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRST";
  key_values[test_k]=test_v;
  int update_len=test_v.size()/2+32;
  std::string new_v1="AgFZJOeRSukFbkinscTxiQmrvKUkEswXziztHDMWitsSQuGjkmCZoTjjXAUGRxalZIWqcgVtTJtAyfUZlxcDZAobSNwlYXAnBaCErzWrJRcWsZEneCvjeJErhnygrwuMDTggKsvHClRdIEPeqOnWagqboEAARBNKCfnaAskbwGQyZdcHVlwszQcOHExregOlIMBlJEWiRFONGBLJjRbBSTMkhJapebTyzVhjgEEnrHNZvyriLlDeNZBqtqMJrgRHKLBvJJUJTExfFQZxDtQUSrTcXBpUijUlIfgRFQxqZBfVkpQkrVywSJHABFLwKrhXZCzkCuNEttUkPsjclQWGqrZjBDsbEdPTIfVdOVNkyDUmIYVgCCaSnMiunfDPdSjWErmSqySJJBccOqTWJGnPXMcCUfdizKyOJRWIJyhHFwxqsZQDyDfpjkBXsLdhPgDdvmxhYRfTnzQyeWPklzGIzKEGiDKUeaUtjtvKkSAkILTkCRRzPglhFanhqqvSCglFjzDPXNthBfTZiafVIiGZcLrMwvqEVxdeHQFYbxgPFVBoTIAolLjZzyhdmTTRtpktnzmQqnzCAkipRVSnIpkVpzGUtPQaZNlSzcnqwLFYxRSTtAuYMkBzISfXijtZUpNcBsvNloMLPqlOBVjdtVzPVKLjSqwIDqpxiqxTGZqlIuRabSONxYVhnPpKDATGJJfCALpmDCxhzbmLRTjoXfammpWHOetolqmCuGqPecuDawsWDLPapoFLKwJwCUcyFzmwpJVJGOgfeSYqlwExMaJDkCDsDKuilLfhdTQWcmaRbVNncftpITjzgXXczXmBOFQdyjXAUAezBdnQsNsGrlcArGjWxcuHBEsZQBdOEzrMXqCowHpEIIRSKRyscCJBCaZjjdmuNosrSJXXsEuwYCUqfJsdYRcVkAzaZzYTeStKcQWdBywKfiShziuUAMlqPIvsJMUTpILWRbpTixvYlRNeLHEkMQiuOUKWaRMhvEHwNVkAPvWpNOKnGULCDEmyMRXGeEZwagUliXBecHmNNuIyTgxdgRfVAtzQ";
  
  client.set(test_k,test_v,"00");



  client.update(test_k,0,new_v1.length(),new_v1);
  key_values[test_k].replace(0,update_len,new_v1);
  std::cout<<"get location sucess"<<std::endl;
  std::cout<<"v1 after update "<<std::endl;
  std::cout<<key_values[test_k];
  std::cout<<std::endl;
  
  /*
  for(auto const &temp_kv:key_values)
  {
    
    auto k1=temp_kv.first;
    if(k1==test_k) continue;
    auto v1=temp_kv.second;
    std::string new_v1;
    std::cout<<"key1:"<<k1<<std::endl;
    std::cout<<"v1.len"<<v1.length();
    std::cout<<"v1 before update "<<std::endl;
    std::cout<<temp_kv.second;
    int update_len=v1.length()/2+32;
    OppoProject::random_generate_value(new_v1,update_len);
    //OppoProject::random_generate_value(new_v1,176);
    std::cout<<"update len: "<<new_v1.length()<<std::endl;
    client.update(k1,0,new_v1.length(),new_v1);
    key_values[k1].replace(0,update_len,new_v1);
    std::cout<<"get location sucess"<<std::endl;

    std::cout<<"v1 after update "<<std::endl;
    std::cout<<key_values[k1];
    std::cout<<std::endl;
  }

  */
  
  
  
  
 /*
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
  */

  
  

  /*
  FILE * fp=fopen("/home/wxh/Documents/oppoEC/OOPPO/log/client.log","a+b");
  std::string temp;
  client.get(test_k,temp);
  std::cout<<key_values[test_k]<<std::endl;
  std::cout<<temp<<std::endl;
  fwrite(key_values[test_k].c_str(),sizeof(char),key_values[test_k].length(),fp);
  std::string ddd="\n";
  fwrite(ddd.c_str(),sizeof(char),ddd.length(),fp);
  fwrite(temp.c_str(),sizeof(char),temp.length(),fp);
  fwrite(ddd.c_str(),sizeof(char),ddd.length(),fp);
  fclose(fp);
  if(temp!=key_values[test_k])
  {
    std::cout<<"G!!!!!!!"<<std::endl;
    
  }
  
  else 
  {
    std::cout<<"6666!!!!!!!!"<<std::endl;
    
  }
  */
  

  /*
  int success=0;
  int failed=0;
  for(auto const &temp_kv:key_values)
  {
    if(test_k==temp_kv.first) continue;
    std::string temp;
    client.get(temp_kv.first,temp);
    std::cout<<"updated value:"<<std::endl;
    std::cout<<temp_kv.second<<std::endl;
    std::cout<<"repaired value:"<<std::endl;
    
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
  */
  

  /*
  for (auto kv : key_values)
  {
    std::string temp;
    client.get(kv.first, temp);
    std::cout<<"repaired value:"<<temp<<std::endl;
    if (temp != kv.second)
    {
      std::cout << temp << std::endl;
      std::cout << "**************************************************************" << std::endl;
      std::cout << kv.second << std::endl;
      std::cout << "**************************************************************" << std::endl;
      std::cout << "repair fail" << std::endl;
    }
    else
    {
      std::cout << "repair success" << std::endl;
    }
  }
  */
  
  //std::string test_v="ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRST";
  //std::string new_v1="AgFZJOeRSukFbkinscTxiQmrvKUkEswXziztHDMWitsSQuGjkmCZoTjjXAUGRxalZIWqcgVtTJtAyfUZlxcDZAobSNwlYXAnBaCErzWrJRcWsZEneCvjeJErhnygrwuMDTggKsvHClRdIEPeqOnWagqboEAARBNKCfnaAskbwGQyZdcHVlwszQcOHExregOlIMBlJEWiRFONGBLJjRbBSTMkhJapebTyzVhjgEEnrHNZvyriLlDeNZBqtqMJrgRHKLBvJJUJTExfFQZxDtQUSrTcXBpUijUlIfgRFQxqZBfVkpQkrVywSJHABFLwKrhXZCzkCuNEttUkPsjclQWGqrZjBDsbEdPTIfVdOVNkyDUmIYVgCCaSnMiunfDPdSjWErmSqySJJBccOqTWJGnPXMcCUfdizKyOJRWIJyhHFwxqsZQDyDfpjkBXsLdhPgDdvmxhYRfTnzQyeWPklzGIzKEGiDKUeaUtjtvKkSAkILTkCRRzPglhFanhqqvSCglFjzDPXNthBfTZiafVIiGZcLrMwvqEVxdeHQFYbxgPFVBoTIAolLjZzyhdmTTRtpktnzmQqnzCAkipRVSnIpkVpzGUtPQaZNlSzcnqwLFYxRSTtAuYMkBzISfXijtZUpNcBsvNloMLPqlOBVjdtVzPVKLjSqwIDqpxiqxTGZqlIuRabSONxYVhnPpKDATGJJfCALpmDCxhzbmLRTjoXfammpWHOetolqmCuGqPecuDawsWDLPapoFLKwJwCUcyFzmwpJVJGOgfeSYqlwExMaJDkCDsDKuilLfhdTQWcmaRbVNncftpITjzgXXczXmBOFQdyjXAUAezBdnQsNsGrlcArGjWxcuHBEsZQBdOEzrMXqCowHpEIIRSKRyscCJBCaZjjdmuNosrSJXXsEuwYCUqfJsdYRcVkAzaZzYTeStKcQWdBywKfiShziuUAMlqPIvsJMUTpILWRbpTixvYlRNeLHEkMQiuOUKWaRMhvEHwNVkAPvWpNOKnGULCDEmyMRXGeEZwagUliXBecHmNNuIyTgxdgRfVAtzQ";

}

/*
//lm test
  
    std::string str="0.0.0.0";
  for (int i = 0; i < 100; i++)
  {
    std::string key;
    std::string value;
    OppoProject::random_generate_kv(key, value, 6, value_length);
    key_values[key] = value;
    std::cout << key.size() << std::endl;
    std::cout << key << std::endl;
    std::cout << value.size() << std::endl;

    

  }
  for(int i=0;i<5;i++)
  {
    for(auto const & t:key_values)
    {
      std::string key=t.first;
      std::string value=t.second;
      client.write_to_logmanager(key.c_str(),key.size(),0,value.c_str(),value.size(),OppoProject::ParityDelta,str.c_str(),49700);
    }

  }
*/




