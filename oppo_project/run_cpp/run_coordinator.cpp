#include "coordinator.h"

int main(int argc, char **argv)
{
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  /*此处需要补充*/
  char buff[256];
  getcwd(buff, 256);
  //std::string config_path = std::string(buff) + "/../../config/AZInformation.xml";
  std::string config_path = "/home/mashuang/ooooppo/OOPPO/oppo_project/config/AZInformation.xml";
  std::cout << "Current working directory: " << config_path << std::endl;
  OppoProject::Coordinator coordinator("10.0.0.10:55555", config_path);
  coordinator.Run();
  return 0;
}