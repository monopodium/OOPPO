#include "client.h"
#include "toolbox.h"
#include "zipf.h"
#include <fstream>
#include <chrono>
#include <fstream>
using namespace std;
using namespace chrono;


int main(int argc, char **argv)
{
  if (argc != 7)
  {
    std::cout << "./run_client Random 3 -1 2 1 99" << std::endl;
    exit(-1);
  }

  OppoProject::PlacementType placement_type;
  if (std::string(argv[1]) == "Random")
  {
    placement_type = OppoProject::Random;
  }
  else if (std::string(argv[1]) == "Best_Placement")
  {
    placement_type = OppoProject::Best_Placement;
  }
  else if (std::string(argv[1]) == "Best_Best_Placement")
  {
    placement_type = OppoProject::Best_Best_Placement;
  }
  else if (std::string(argv[1]) == "Best_Best_Best_Placement")
  {
    placement_type = OppoProject::Best_Best_Best_Placement;
  }
  else if (std::string(argv[1]) == "Par_2_random")
  {
    placement_type = OppoProject::Par_2_random;
  }
  else if (std::string(argv[1]) == "Par_2_load")
  {
    placement_type = OppoProject::Par_2_load;
  }
  else
  {
    std::cout << "error: unknown placement_type" << std::endl;
    exit(-1);
  }
  int k = std::stoi(std::string(argv[2]));
  int real_l = std::stoi(std::string(argv[3]));
  int b = std::ceil((double)k / (double)real_l);
  int g_m = std::stoi(std::string(argv[4]));
  int block_size = std::stoi(std::string(argv[5])); //单位KB
  double u = (double)(std::stoi(std::string(argv[6]))) / 100;
  std::cout << "u: " << u << std::endl;
  int num_of_nodes = 200;
  int value_length = block_size * 1024 * k;
  int num_of_stripes = 1 * 1024 * 1024 / (value_length / 1024);
  std::cout << num_of_stripes << " " << block_size << " " << value_length << std::endl;


  OppoProject::Client client(std::string("10.0.0.10"), 44444, std::string("10.0.0.10:55555"));
  std::cout << client.sayHelloToCoordinatorByGrpc("MMMMMMMM") << std::endl;
  if (client.SetParameterByGrpc({false, OppoProject::Azure_LRC, placement_type, k, real_l, g_m, b, 0, 2147483647}))
  {
    std::cout << "set parameter successfully!" << std::endl;
  }
  else
  {
    std::cout << "Failed to set parameter!" << std::endl;
  }

  // close(STDOUT_FILENO);
  // close(STDERR_FILENO);
  int read    =   72000;
  int d_read  =   18000;
  int write   =    9000;
  int repair =     1000;
  int index = 0;
  std::unordered_map<int, std::string> all_keys_with_idx;
  std::unordered_set<std::string> all_keys;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<unsigned int> dis(0, num_of_nodes - 1);

  for (int i = 0; i < 100; i++) {
    std::cout << "test progress#######: " << i << std::endl;
    for (int j = 0; j < write / 100; j++) {
        std::string key, value;
        OppoProject::gen_key_value(all_keys, 50, key, 1024, value);
        all_keys_with_idx[index++] = key;
        all_keys.insert(key);
        client.set(key, value, "00");
    }
    std::default_random_engine generator;
    zipfian_int_distribution<int> distribution(0, all_keys.size() - 1, u);
    for (int j = 0; j < d_read / 100; j++) {
        int idx = distribution(generator);
        std::string temp;
        idx = all_keys.size() - 1 - idx;
        client.get(all_keys_with_idx[idx], temp);
    }
    for (int j = 0; j < read / 100; j++) {
        int idx = distribution(generator);
        std::string temp;
        idx = all_keys.size() - 1 - idx;
        client.get(all_keys_with_idx[idx], temp);
    }
    for (int j = 0; j < repair / 100; j++) {
      int node_id = dis(gen);
      std::vector<int> failed_node_list = {node_id};
      client.repair(failed_node_list);
    }
  }

  string test_result_file = "/home/mashuang/ooooppo/OOPPO/oppo_project/test.result";
  ofstream fout(test_result_file, std::ios::app);
  double node_storage_bias;
  double node_network_bias;
  double az_storage_bias;
  double az_network_bias;
  double cross_repair_traffic;
  double degraded_time;
  double all_time;
  client.checkBias(node_storage_bias, node_network_bias, az_storage_bias, az_network_bias, cross_repair_traffic, degraded_time, all_time);
  fout << "node_storage_bias: " << node_storage_bias << std::endl;
  fout << "node_network_bias: " << node_network_bias << std::endl;
  fout << "az_storage_bias: " << az_storage_bias << std::endl;
  fout << "az_network_bias: " << az_network_bias << std::endl;
  fout.close();
}
