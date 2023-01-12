#include "proxy.h"

int main(int argc, char** argv){
    char buff[256];
    getcwd(buff, 256);
    std::string config_path = std::string(buff) + "/../../config/AZInformation.xml";
    OppoProject::Proxy proxy(std::string("0.0.0.0:50055"), config_path);
    proxy.Run();
    return 0;
}