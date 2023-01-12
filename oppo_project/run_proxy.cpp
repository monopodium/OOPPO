#include "proxy.h"

int main(int argc, char** argv){
   
    OppoProject::Proxy proxy(std::string("0.0.0.0:50055"));
    proxy.Run();
    return 0;
}