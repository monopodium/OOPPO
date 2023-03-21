#include "coordinator.h"

int main(int argc, char **argv)
{
  std::string coordinator_ip = "0.0.0.0";
  if (argc == 2) {
    coordinator_ip = std::string(argv[1]);
  }
  /*此处需要补充*/
  char buff[256];
  getcwd(buff, 256);
  std::string config_path = std::string(buff) + "/../../config/AZInformation.xml";
  std::cout << "Current working directory: " << config_path << std::endl;
  OppoProject::Coordinator coordinator(coordinator_ip + ":55555", config_path);
  coordinator.Run();
  return 0;
}