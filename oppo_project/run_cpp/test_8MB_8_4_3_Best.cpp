#include "client.h"
#include "toolbox.h"
#include <fstream>
#include <chrono>
#include <fstream>
using namespace std;
using namespace chrono;


int main(int argc, char **argv)
{
  int k = 8;
  int real_l = 4;
  int g_m = 3;
  int b = k / real_l;
  OppoProject::Client client(std::string("10.0.0.10"), 44444, std::string("10.0.0.10:55555"));
  std::cout << client.sayHelloToCoordinatorByGrpc("MMMMMMMM") << std::endl;
  if (client.SetParameterByGrpc({false, OppoProject::Azure_LRC, OppoProject::Best_Placement, k, real_l, g_m, b, 0, 2147483647}))
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
  int num_of_stripes = 20;
  int num_of_nodes = 120;
  int block_size = 8; //单位MB
  int value_length = block_size * 1024 * 1024 * k;
  for (int i = 0; i < num_of_stripes; i++)
  {
      std::cout << "progress: " << i << std::endl;
      std::string key;
      std::string value;
      OppoProject::gen_key_value(keys, 50, key, value_length, value);
      key_values[key] = value;
      keys.insert(key);
      client.set(key, value, "00");
  }
  std::cout << "开始修复" << std::endl;
  auto start = system_clock::now();
  for (int j = 0; j < num_of_nodes; j++)
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
  double time_cost = double(duration.count()) * microseconds::period::num / microseconds::period::den;
  fout <<  "时间开销：" << time_cost << "秒" << endl;
  double repair_speed = (double)num_of_stripes * (double)(k + real_l + g_m) * (double)block_size / time_cost;
  fout <<  "修复速度：" << repair_speed << "MB/秒" << endl;
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
