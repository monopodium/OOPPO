#include "client.h"
#include "toolbox.h"
#include <fstream>

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
  else if (std::string(argv[2]) == "Azure_LRC") {
    encode_type = OppoProject::Azure_LRC;
  }
  else
  {
    std::cout << "error: unknown encode_type" << std::endl;
    exit(-1);
  }
  std::cout << std::string(argv[2]) << std::endl;
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

  OppoProject::Client client(std::string("0.0.0.0"), 44444, std::string("0.0.0.0:55555"));
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

  std::cout << value_length << std::endl;
  std::unordered_set<std::string> all_keys;
  std::unordered_map<std::string, std::string> kvs;
  int w = 100;
  std::string value(value_length, '#');
  for (int i = 0; i < w; i++) {
    std::string key = OppoProject::gen_key(50, all_keys);
    OppoProject::random_generate_value(value, value_length);
    all_keys.insert(key);
    kvs[key] = value;
    client.set(key, value, "00");
  }

  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 20; j++) {
      int temp = 9000 + i * 100 + j;
      std::cout << "repair: " << temp << std::endl;
      std::vector<std::string> failed_node_list={std::to_string(temp)};
      client.repair(failed_node_list);
    }
  }

  for (auto &p : kvs) {
    std::string temp_value;
    client.get(p.first, temp_value);
    if (p.second != temp_value) {
      std::cout << p.second << std::endl;
      std::cout << temp_value << std::endl;
      std::cout << "repair fail" << std::endl;
      // break;
    } else {
      std::cout << "repair successfully" << std::endl;
    }
  }

}