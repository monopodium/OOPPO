#include "proxy.h"

int main(int argc, char **argv)
{
<<<<<<< HEAD
    /*
=======
    std::string coordinator_ip = "0.0.0.0";
    if (argc == 3) {
        coordinator_ip = std::string(argv[2]);
    }
>>>>>>> 82971bb7664c255e1550c0248037fd342ccca0f2
    pid_t pid = fork();
    if (pid > 0)
    {
        exit(0);
    }
    setsid();
<<<<<<< HEAD
    chdir("~/OOPPO/");
<<<<<<< HEAD
    umask(0);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    */
=======
=======
>>>>>>> 82971bb7664c255e1550c0248037fd342ccca0f2
    
>>>>>>> dd9b2acb3debf74b5046fa544668da36d6ef0aab
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