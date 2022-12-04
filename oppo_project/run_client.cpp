#include "client.h"

int main(int argc, char** argv){
    // if(argc != 6) {
    //     std::cout<<"./client k l g r placement block_size[KB] stripe_number !"<<std::endl<<std::flush;
    //     std::cout<<"r"<<std::endl<<std::flush;
    //     std::cout<<"c"<<std::endl<<std::flush;
    //     std::cout<<"placement"<<std::endl<<std::flush; //1:Random 2:DIS 3:AGG
    //     exit(1);
    // }    
    OppoProject::Client client;
    std::cout << client.sayHelloToCoordinatorByGrpc("MMMMMMMM") << std::endl;
    return 0;
}