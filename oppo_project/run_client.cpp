#include "client.h"
#include "toolbox.h"
#include <fstream>
#include <queue>
#include <random>
#include "zipf.h"

#include <assert.h>             // Needed for assert() macro
#include <stdio.h>              // Needed for printf()
#include <stdlib.h>             // Needed for exit() and ato*()
#include <math.h>               // Needed for pow()


int main(int argc, char **argv) {
  if (argc != 11) {
    std::cout << "./run_client partial_decoding encode_type placement_type k real_l g small_file_upper blob_size_upper trace value_length" << std::endl;
    std::cout << "./run_client false RS Flat 3 -1 2 1024 4096 random 2048" << std::endl;
    exit(-1);
  }

  bool partial_decoding;
  OppoProject::EncodeType encode_type;
  OppoProject::PlacementType placement_type;
  int k, real_l, g_m, b;
  int small_file_upper, blob_size_upper;
  int value_length;

  partial_decoding = (std::string(argv[1]) == "true");
  if (std::string(argv[2]) == "OPPO_LRC") {
    encode_type = OppoProject::OPPO_LRC;
  } else if (std::string(argv[2]) == "Azure_LRC_1") {
    encode_type = OppoProject::Azure_LRC_1;
  } else if (std::string(argv[2]) == "RS") {
    encode_type = OppoProject::RS;
  } else {
    std::cout << "error: unknown encode_type" << std::endl;
    exit(-1);
  }
  if (std::string(argv[3]) == "Flat") {
    placement_type = OppoProject::Flat;
  } else if (std::string(argv[3]) == "Random") {
    placement_type = OppoProject::Random;
  } else if (std::string(argv[3]) == "Best_Placement") {
    placement_type = OppoProject::Best_Placement;
  } else {
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

  OppoProject::Client client(std::string("0.0.0.0"), 44444, std::string("0.0.0.0:55555"));
  /**测试**/
  std::cout << client.sayHelloToCoordinatorByGrpc("MMMMMMMM") << std::endl;
  /**测试**/

  /**设置编码参数的函数，咱就是说有用没用都给传过去存下来，
   * 现在的想法就是每次需要修改这个参数，都要调用一次这个函数来改**/

  if (client.SetParameterByGrpc({partial_decoding, encode_type, placement_type, k, real_l, g_m, b, small_file_upper, blob_size_upper})) {
    std::cout << "set parameter successfully!" << std::endl;
  } else {
    std::cout << "Failed to set parameter!" << std::endl;
  }

  std::string value(value_length, '@');
  std::vector<std::string> recent_keys;
  std::unordered_map<int, std::string> all_keys_with_idx;
  std::unordered_set<std::string> all_keys;
  std::unordered_map<int, int> check;
  std::mutex m_mutex;
  int index = 0;
  int w1 = 100, w2 = 475000;
  // for (; index < w1; index++) {
  //   std::string key = OppoProject::gen_key(50, all_keys);
  //   all_keys_with_idx[index] = key;
  //   all_keys.insert(key);
  //   client.set(key, value, "00");
  // }
  // std::cout << "热身完毕" << std::endl;

  for (int i = 0; i < 250; i++) {
    std::cout << "****************  " << i << "  ********************" << std::endl;
    for (int j = 0; j < w1; j++) {
      std::string key = OppoProject::gen_key(50, all_keys);
      all_keys_with_idx[index++] = key;
      all_keys.insert(key);
      client.set(key, value, "00");
    }
    std::default_random_engine generator;
    zipfian_int_distribution<int> distribution(0, all_keys.size() - 1, 0.99);
    for (int j = 0; j < 1900; j++) {
      int idx = distribution(generator);
      std::string temp;
      idx = all_keys.size() - 1 - idx;
      check[idx]++;
      client.get(all_keys_with_idx[idx], temp);
    }
  }
  // for (int j = 0; j < w1; j++) {
  //   std::string key = OppoProject::gen_key(50, all_keys);
  //   all_keys_with_idx[index++] = key;
  //   all_keys.insert(key);
  //   client.set(key, value, "00");
  // }

  for (auto p : check) {
    if (p.second > 1500) {
      std::cout << p.first << " " << p.second << std::endl;
    }
  }


  // std::default_random_engine generator1;
  // zipfian_int_distribution<int> distribution1(0, all_keys.size() - 1, 0.99);
  // std::unordered_map<int, int> check;
  // for (int i = 0; i < w2; i++) {
  //   int idx = distribution1(generator1);
  //   std::string temp;
  //   idx = all_keys.size() - 1 - idx;
  //   check[idx]++;
  //   client.get(all_keys_with_idx[idx], temp);
  // }
  // for (auto p : check) {
  //   if (p.second > 500) {
  //     std::cout << p.first << " " << p.second << std::endl;
  //   }
  // }
  // for (int i = 0; i < w1; i++, index++) {
    // std::string key = OppoProject::gen_key(50, all_keys);
    // all_keys_with_idx[index] = key;
    // all_keys.insert(key);
    // client.set(key, value, "00");
  // }

  // rand_val(2);
  // for (int i = 0; i < w2; i++) {
  //   int idx = get_idx(all_keys.size() - 1);
  //   std::string temp;
  //   idx = all_keys.size() - 1 - idx;
  //   client.get(all_keys_with_idx[idx], temp);
  // }
  // for (int i = 0; i < w1; i++, index++) {
  //   std::string key = OppoProject::gen_key(50, all_keys);
  //   all_keys_with_idx[index] = key;
  //   all_keys.insert(key);
  //   client.set(key, value, "00");
  // }

  // rand_val(3);
  // for (int i = 0; i < w2; i++) {
  //   int idx = get_idx(all_keys.size() - 1);
  //   std::string temp;
  //   idx = all_keys.size() - 1 - idx;
  //   client.get(all_keys_with_idx[idx], temp);
  // }
  // for (int i = 0; i < w1; i++, index++) {
  //   std::string key = OppoProject::gen_key(50, all_keys);
  //   all_keys_with_idx[index] = key;
  //   all_keys.insert(key);
  //   client.set(key, value, "00");
  // }

  // rand_val(4);
  // for (int i = 0; i < w2; i++) {
  //   int idx = get_idx(all_keys.size() - 1);
  //   std::string temp;
  //   idx = all_keys.size() - 1 - idx;
  //   client.get(all_keys_with_idx[idx], temp);
  // }
  // for (int i = 0; i < w1; i++, index++) {
  //   std::string key = OppoProject::gen_key(50, all_keys);
  //   all_keys_with_idx[index] = key;
  //   all_keys.insert(key);
  //   client.set(key, value, "00");
  // }



  // std::thread th1 = std::thread([&](){
  //   std::unordered_map<int, int> help;
  //   for (int i = 0; i < w1; i++) {
  //     m_mutex.lock();
  //     int idx = get_idx(w1 + w2 - 1);
  //     std::string temp;
  //     if (idx >= all_keys.size()) {
  //       idx %= all_keys.size(); 
  //     }
  //     idx = all_keys.size() - 1 - idx;
  //     help[idx]++;
  //     client.get(all_keys_with_idx[idx], temp);
  //     m_mutex.unlock();
  //   }
  //   for (auto &p : help) {
  //     if (p.second > 100) {
  //       std::cout << p.first << " " << p.second << std::endl;
  //     }
  //   }
  // });
  // std::thread th2 = std::thread([&](){
  //   for (int j = 0; j < w2; j++, index++) {
  //     m_mutex.lock();
  //     std::string key = OppoProject::gen_key(50, all_keys);
  //     all_keys_with_idx[index] = key;
  //     all_keys.insert(key);
  //     client.set(key, value, "00");
  //     m_mutex.unlock();
  //   }
  // });
  // th1.join();
  // th2.join();



  // std::unordered_map<std::string, std::string> key_values;
  // /*生成随机的key value对*/
  
  // if(std::string(argv[9]) == "random"){
  //   for (int i = 0; i < 1000; i++) {
  //     std::string key;
  //     std::string value;
  //     OppoProject::random_generate_kv(key, value, 6, value_length);
  //     key_values[key] = value;
  //     std::cout << key.size() << std::endl;
  //     std::cout << key << std::endl;
  //     std::cout << value.size() << std::endl;

  //     client.set(key, value, "00");

  //     std::string get_value;
  //     client.get(key, get_value);

  //     // if (value == get_value) {
  //     //   std::cout << "set kv successfully" << std::endl;
  //     // } else {
  //     //   std::cout << "wrong!" << std::endl;
  //     //   break;
  //     // }
  //   }
  // } else if (std::string(argv[9]) == "ycsb"){
  //   std::string line;
  //   char inst;
  //   std::string key;
  //   std::string warm_path = "/home/ptbxzrt/YCSB-tracegen/warm.txt";
  //   std::string test_path = "/home/ptbxzrt/YCSB-tracegen/test.txt";
  //   std::ifstream inf;
  //   for(int p=0;p<2;p++){
  //     if(p==0) 
  //       inf.open(warm_path);
  //     else
  //       inf.open(test_path);
  //     if(!inf)
  //       std::cerr<<"cannot open the file"<<std::endl;
  //     while(getline(inf,line)){
  //       const char *delim = " ";
  //       inst = line[0];
  //       int pos = line.find(delim,2);
  //       key = line.substr(2,pos-2);
  //       if(inst == 'I'){
  //           std::string value;
  //           OppoProject::random_generate_value(value,value_length);
  //           key_values[key] = value;
  //           // std::cout << key.size() << std::endl;
  //           // std::cout << key << std::endl;
  //           // std::cout << value.size() << std::endl;
  //           client.set(key, value, "00");
  //       }else if (inst == 'R'){
  //         std::string get_value;
  //         client.get(key, get_value);
  //         // std::cout << key.size() << std::endl;
  //         // std::cout << key << std::endl;
  //         // std::cout << get_value.size() << std::endl;
  //         if (key_values[key] == get_value) {
  //           std::cout << "set kv successfully" << std::endl;
  //         } else {
  //           std::cout << key_values[key] << std::endl;
  //           std::cout << get_value << std::endl;
  //           std::cout << "wrong!" << std::endl;
  //           break;
  //         }
  //       }
  //     } 
  //     inf.close();
  //   }
  // } 
  std::cout << "set/get finish!" << std::endl;
  client.checkBias();

  

  // std::cout << "开始修复" << std::endl;
  // // 这里其实应该用ip
  // // 但目前我们是在单机上进行测试的，所以暂时用端口号代替一下
  // for (int i = 0; i < 10; i++) {
  //   for (int j = 0; j < 10; j++) {
  //     int temp = 9000 + i * 100 + j;
  //     std::cout << "repair" << temp << std::endl;
  //     std::vector<std::string> failed_node_list={std::to_string(temp)};
  //     client.repair(failed_node_list);
  //   }
  // }

  // for (auto kv : key_values) {
  //   std::string temp;
  //   client.get(kv.first, temp);
  //   if (temp != kv.second) {
  //     std::cout << temp << std::endl;
  //     std::cout << "**************************************************************" << std::endl;
  //     std::cout << kv.second << std::endl;
  //     std::cout << "**************************************************************" << std::endl;
  //     std::cout << "repair fail" << std::endl;
  //   } else {
  //     std::cout << "repair success" << std::endl;
  //   }
  // }

}