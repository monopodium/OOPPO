#include "client.h"
#include "toolbox.h"

int main(int argc, char **argv) {
  if (argc != 9) {
    std::cout << "./run_client partial_decoding encode_type placement_type k l g small_file_upper blob_size_upper" << std::endl;
    std::cout << "./run_client false RS Flat 3 -1 2 1024 4096" << std::endl;
    exit(-1);
  }

  bool partial_decoding;
  OppoProject::EncodeType encode_type;
  OppoProject::PlacementType placement_type;
  int k, l, g_m, r;
  int small_file_upper, blob_size_upper;

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
  l = std::stoi(std::string(argv[5]));
  r = k / l;
  g_m = std::stoi(std::string(argv[6]));
  small_file_upper = std::stoi(std::string(argv[7]));
  blob_size_upper = std::stoi(std::string(argv[8]));


  OppoProject::Client client(std::string("0.0.0.0"), 44444, std::string("0.0.0.0:55555"));
  /**测试**/
  std::cout << client.sayHelloToCoordinatorByGrpc("MMMMMMMM") << std::endl;
  /**测试**/

  /**设置编码参数的函数，咱就是说有用没用都给传过去存下来，
   * 现在的想法就是每次需要修改这个参数，都要调用一次这个函数来改**/

  if (client.SetParameterByGrpc({partial_decoding, encode_type, placement_type, k, l, g_m, r, small_file_upper, blob_size_upper})) {
    std::cout << "set parameter successfully!" << std::endl;
  } else {
    std::cout << "Failed to set parameter!" << std::endl;
  }

  /*生成随机的key value对*/
  for (int i = 0; i < 1000; i++) {
    std::string key;
    std::string value;
    OppoProject::random_generate_kv(key, value, 6, 1600);
    std::cout << key.size() << std::endl;
    std::cout << key << std::endl;
    std::cout << value.size() << std::endl;
    std::cout << value << std::endl;

    client.set(key, value, "00");

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