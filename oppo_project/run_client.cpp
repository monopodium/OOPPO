#include "client.h"
#include "toolbox.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << "./client encodeType" << std::endl;
    exit(1);
  }

  /*需要使用命令行传入的参数部分*/
  std::string input_encode_type = argv[1];

  std::cout << "encodeType:" << input_encode_type << std::endl;
  int k = 3;
  int l = 0; //如果一个编码用不到某个变量，就设为0
  int r = 0;
  int g_m = 2;
  bool if_partial_decoding = false;
  std::string input_placement_type = "Flat";
  /**需要使用命令行传入的参数部分**/

  OppoProject::EncodeType encode_type;
  OppoProject::PlacementType placement_type;
  if (input_encode_type == "OPPO_LRC") {
    encode_type = OppoProject::OPPO_LRC;
  } else if (input_encode_type == "Azure_LRC_1") {
    encode_type = OppoProject::Azure_LRC_1;
  } else {
    encode_type = OppoProject::RS;
  }

  if (input_placement_type == "Random") {
    placement_type = OppoProject::Random;
  } else if (input_placement_type == "Best_Placement") {
    placement_type = OppoProject::Best_Placement;
  } else {
    placement_type = OppoProject::Flat;
  }

  OppoProject::Client client;
  /**测试**/
  std::cout << client.sayHelloToCoordinatorByGrpc("MMMMMMMM") << std::endl;
  /**测试**/

  /**设置编码参数的函数，咱就是说有用没用都给传过去存下来，
   * 现在的想法就是每次需要修改这个参数，都要调用一次这个函数来改**/

  if (client.SetParameter({encode_type, placement_type, k, l, g_m, r})) {
    std::cout << "set parameter successfully!" << std::endl;
  } else {
    std::cout << "Failed to set parameter!" << std::endl;
  }

  /*生成随机的key value对*/
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
}