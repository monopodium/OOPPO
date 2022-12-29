#include "coordinator.h"

int main(int argc, char **argv) {

  /*此处需要补充*/
  OppoProject::Coordinator coordinator(
      "0.0.0.0:50051", "/OOPPO/oppo_project/config/AZInformation.xml");
  coordinator.Run();
  return 0;
}