#include "proxy.h"

int main(int argc, char **argv)
{
    std::string coordinator_ip = "0.0.0.0";
    if (argc == 3) {
        coordinator_ip = std::string(argv[2]);
    }
    pid_t pid = fork();
    if (pid > 0)
    {
        exit(0);
    }
    setsid();
    
    std::string ip_and_port(argv[1]);
    if(false){
        umask(0);
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }

    char buff[256];
    getcwd(buff, 256);
    std::string config_path = std::string(buff) + "/../../config/AZInformation.xml";
    OppoProject::Proxy proxy(ip_and_port, config_path, coordinator_ip);
    proxy.Run();
    return 0;
}