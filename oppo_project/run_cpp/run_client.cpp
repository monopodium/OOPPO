#include "client.h"
#include "toolbox.h"
#include <fstream>
#include <chrono>
#include <fstream>
using namespace std;
using namespace chrono;


int main(int argc, char **argv)
{
  if (argc != 11)
  {
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
  else if (std::string(argv[2]) == "Azure_LRC")
  {
    encode_type = OppoProject::Azure_LRC;
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
  else if (std::string(argv[3]) == "Best_Best_Placement")
  {
    placement_type = OppoProject::Best_Best_Placement;
  }
  else if (std::string(argv[3]) == "Best_Best_Best_Placement")
  {
    placement_type = OppoProject::Best_Best_Best_Placement;
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

  OppoProject::Client client(std::string("10.0.0.10"), 44444, std::string("10.0.0.10:55555"));
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

  // close(STDOUT_FILENO);
  // close(STDERR_FILENO);
  std::unordered_map<std::string, std::string> key_values;
  std::unordered_set<std::string> keys;
  /*生成随机的key value对*/

  if (std::string(argv[9]) == "random")
  {
    for (int i = 0; i < 20; i++)
    {
      std::cout << "progress: " << i << std::endl;
      std::string key;
      std::string value;
      OppoProject::gen_key_value(keys, 50, key, value_length, value);
      key_values[key] = value;
      keys.insert(key);
      client.set(key, value, "00");
    }
  }
  std::cout << "开始修复" << std::endl;
  auto start = system_clock::now();
  for (int j = 0; j < 120; j++)
  {
    int temp = j;
    std::cout << "repair: " << temp << std::endl;
    std::vector<int> failed_node_list = {temp};
    client.repair(failed_node_list);
  }
  auto end   = system_clock::now();
  auto duration = duration_cast<microseconds>(end - start);
  string test_result_file = "/home/mashuang/ooooppo/OOPPO/oppo_project/test.result";
  ofstream fout(test_result_file, ios::out | ios::trunc);
  for (auto kv : key_values)
  {
    std::string temp;
    client.get(kv.first, temp);
    if (temp != kv.second)
    {
      fout << "repair fail" << std::endl;
      break;
    }
    else
    {
      std::cout << "repair success" << std::endl;
    }
  }
  fout <<  "花费了" << double(duration.count()) * microseconds::period::num / microseconds::period::den << "秒" << endl;
  double node_storage_bias;
  double node_network_bias;
  double az_storage_bias;
  double az_network_bias;
  double cross_repair_traffic;
  client.checkBias(node_storage_bias, node_network_bias, az_storage_bias, az_network_bias, cross_repair_traffic);
  fout << "node_storage_bias: " << node_storage_bias << std::endl;
  fout << "node_network_bias: " << node_network_bias << std::endl;
  fout << "az_storage_bias: " << az_storage_bias << std::endl;
  fout << "az_network_bias: " << az_network_bias << std::endl;
  fout << "cross_repair_traffic: " << cross_repair_traffic << std::endl;
  fout.close();
}
