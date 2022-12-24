#include "client.h"
#include <fstream>
using namespace std;
int main(int argc, char **argv) {
  // if(argc != 6) {
  //     std::cout<<"./client k l g r placement block_size[KB] stripe_number
  //     !"<<std::endl<<std::flush; std::cout<<"r"<<std::endl<<std::flush;
  //     std::cout<<"c"<<std::endl<<std::flush;
  //     std::cout<<"placement"<<std::endl<<std::flush; //1:Random 2:DIS 3:AGG
  //     exit(1);
  // }
  OppoProject::EncodeType input_encode_type = OppoProject::RS;
  OppoProject::Client client;
  std::cout << client.sayHelloToCoordinatorByGrpc("MMMMMMMM") << std::endl;
  std::ifstream infile;
  char data[2048];

  infile.open("afile.dat");
  std::string value = "A"; // = data;
  // infile >> data;
  for (int i = 0; i < 60; i++) {
    for (int j = 65; j < 90; j++) {
      value = value + char(j);
    }
  }

  std::cout << value.size() << std::endl;
  std::cout << value << std::endl;

  std::string key = "KKKKKK";
  client.set(key, value, "00");
}