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

  std::unordered_map<std::string, std::string> key_values;
  /*生成随机的key value对*/

  if (std::string(argv[9]) == "random")
  {
    for (int i = 0; i < 1000; i++)
    {
      std::string key;
      std::string value;
      OppoProject::random_generate_kv(key, value, 6, value_length);
      key_values[key] = value;
      std::cout << key.size() << std::endl;
      std::cout << key << std::endl;
      std::cout << value.size() << std::endl;

      client.set(key, value, "00");

      std::string get_value;
      client.get(key, get_value);

      // if (value == get_value) {
      //   std::cout << "set kv successfully" << std::endl;
      // } else {
      //   std::cout << "wrong!" << std::endl;
      //   break;
      // }
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
          client.get(key, get_value);
          std::cout << key.size() << std::endl;
          std::cout << key << std::endl;
          std::cout << get_value.size() << std::endl;
          if (key_values[key] == get_value)
          {
            std::cout << "set kv successfully" << std::endl;
          }
          else
          {
            std::cout << "wrong!" << std::endl;
            break;
          }
        }
      }
      inf.close();
    }
  }
  std::cout << "set/get finish!" << std::endl;

  std::cout << "开始修复" << std::endl;
  // 这里其实应该用ip
  // 但目前我们是在单机上进行测试的，所以暂时用端口号代替一下
  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 10; j++)
    {
      int temp = 9000 + i * 100 + j;
      std::cout << "repair" << temp << std::endl;
      std::vector<std::string> failed_node_list = {std::to_string(temp)};
      client.repair(failed_node_list);
    }
  }
  for (int i = 0; i < 10; i++)
  {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned int> dis(0, 9);
    int p = dis(gen);
    int q;
    do
    {
      q = dis(gen);
    } while (p == q);
    int temp1 = 9000 + i * 100 + 0;
    int temp2 = 9000 + i * 100 + 1;
    int temp3 = 9000 + i * 100 + 6;
    int temp4 = 9000 + i * 100 + 9;
    int temp5 = 9000 + i * 100 + 4;
    int temp6 = 9000 + i * 100 + 3;
    std::cout << "repair: temp1 " << temp1 << ", temp2 " << temp2 << std::endl;
    std::vector<std::string> failed_node_list = {std::to_string(temp1), std::to_string(temp2), std::to_string(temp3), std::to_string(temp4) /*, std::to_string(temp5), std::to_string(temp6)*/};
    client.repair(failed_node_list);
  }

  for (auto kv : key_values)
  {
    std::string temp;
    client.get(kv.first, temp);
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
}