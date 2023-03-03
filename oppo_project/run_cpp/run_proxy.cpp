#include "proxy.h"

int main(int argc, char **argv)
{
    pid_t pid = fork();
    if (pid > 0)
    {
        exit(0);
    }
    setsid();
    chdir("~/OOPPO/");
    umask(0);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    std::string ip_and_port(argv[1]);

    char buff[256];
    getcwd(buff, 256);
    std::string config_path = std::string(buff) + "/../../config/AZInformation.xml";
    OppoProject::Proxy proxy(ip_and_port, config_path);
    proxy.Run();
    return 0;
}