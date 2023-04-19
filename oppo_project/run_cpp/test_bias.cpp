#include "client.h"
#include "toolbox.h"
#include "zipf.h"
#include <fstream>
#include <chrono>
#include <fstream>
using namespace std;
using namespace chrono;

bool cmp_idx(std::pair<int, int> &a, std::pair<int, int> &b) {
  return a.first < b.first;
}

int main(int argc, char **argv)
{
  // if (argc != 8 || argc != 7)
  // {
  //   std::cout << "./run_client Random 3 -1 2 1 99 50" << std::endl;
  //   exit(-1);
  // }

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
  int alpha = std::stoi(std::string(argv[7]));
  std::cout << "u: " << u << std::endl;
  int num_of_nodes = 200;
  int value_length = block_size * 1024 * k;
  int num_of_stripes = 1 * 1024 * 1024 / (value_length / 1024);
  std::cout << num_of_stripes << " " << block_size << " " << value_length << std::endl;


  OppoProject::Client client(std::string("10.0.0.10"), 44444, std::string("10.0.0.10:55555"));
  std::cout << client.sayHelloToCoordinatorByGrpc("MMMMMMMM") << std::endl;
  if (client.SetParameterByGrpc({false, OppoProject::Azure_LRC, placement_type, k, real_l, g_m, b, 0, 2147483647}, alpha))
  {
    std::cout << "set parameter successfully!" << std::endl;
  }
  else
  {
    std::cout << "Failed to set parameter!" << std::endl;
  }

  // close(STDOUT_FILENO);
  // close(STDERR_FILENO);
  int read    =   95000 * 5;
  int write   =    5000 * 5;
  int index = 0;
  std::unordered_map<int, std::string> all_keys_with_idx;
  std::unordered_set<std::string> all_keys;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<unsigned int> dis(0, num_of_nodes - 1);
  std::unordered_map<int, int> records;

  double once_write = 1;
  double beli = ((double)read / (double)write) * (2 * once_write / ((double)write + once_write));
  int diedai = write / once_write;
  int read_count = 0;
  int repair_count = 0;

  for (int i = 0; i < diedai; i++) {
    std::cout << "test progress#######: " << i << std::endl;
    for (int j = 0; j < once_write; j++) {
        std::string key, value;
        OppoProject::gen_key_value(all_keys, 50, key, 1024, value);
        all_keys_with_idx[index++] = key;
        all_keys.insert(key);
        client.set(key, value, "00");
    }
    // int total_read = all_keys.size() * beli;
    std::default_random_engine generator;
    zipfian_int_distribution<int> distribution(0, all_keys.size() - 1, u);
    for (int j = 0; j < std::ceil(((double)read / (double)diedai) * 0.8); j++) {
        read_count++;
        int idx = distribution(generator);
        std::string temp;
        idx = all_keys.size() - 1 - idx;
        records[idx]++;
        client.get(all_keys_with_idx[idx], temp);
    }
    for (int j = 0; j < std::ceil(((double)read / (double)diedai) * 0.2); j++) {
        read_count++;
        int idx = distribution(generator);
        std::string temp;
        idx = all_keys.size() - 1 - idx;
        records[idx]++;
        client.simulate_d_read(all_keys_with_idx[idx], temp);
    }
    // for (int j = 0; j < (write / 1000); j++) {
    //   repair_count++;
    //   int node_id = dis(gen);
    //   std::vector<int> failed_node_list = {node_id};
    //   client.repair(failed_node_list);
    // }
  }
  int node_id = dis(gen);
  std::vector<int> failed_node_list = {node_id};
  client.repair(failed_node_list);

  // for (int i = 0; i < diedai; i++) {
  //   std::cout << "test progress#######: " << i << std::endl;
  //   for (int j = 0; j < write / diedai; j++) {
  //       std::string key, value;
  //       OppoProject::gen_key_value(all_keys, 50, key, 1024, value);
  //       all_keys_with_idx[index++] = key;
  //       all_keys.insert(key);
  //       client.set(key, value, "00");
  //   }
  //   std::default_random_engine generator;
  //   zipfian_int_distribution<int> distribution(0, all_keys.size() - 1, u);
  //   for (int j = 0; j < d_read / diedai; j++) {
  //       int idx = distribution(generator);
  //       std::string temp;
  //       idx = all_keys.size() - 1 - idx;
  //       records[idx]++;
  //       client.simulate_d_read(all_keys_with_idx[idx], temp);
  //   }
  //   for (int j = 0; j < read / diedai; j++) {
  //       int idx = distribution(generator);
  //       std::string temp;
  //       idx = all_keys.size() - 1 - idx;
  //       records[idx]++;
  //       client.get(all_keys_with_idx[idx], temp);
  //   }
  //   for (int j = 0; j < repair / diedai; j++) {
  //     int node_id = dis(gen);
  //     std::vector<int> failed_node_list = {node_id};
  //     client.repair(failed_node_list);
  //   }
  // }
  std::vector<std::pair<int, int>> help;
  for (auto p : records) {
    help.push_back({p.first, p.second});
  }
  sort(help.begin(), help.end(), cmp_idx);
  for (auto p : help) {
    std::cout << p.first << ": " << p.second << std::endl;
  }
  std::cout << once_write << " " << repair_count << " " << diedai << " " << read_count << std::endl;


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
