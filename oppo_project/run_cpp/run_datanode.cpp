#include "datanode.h"

int main(int argc, char **argv)
{
    pid_t pid = fork();
    if (pid > 0)
    {
        exit(0);
    }
    setsid();
    umask(0);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    std::string ip_and_port(argv[1]);
    std::string ip = ip_and_port.substr(0, ip_and_port.find(":"));
    int port = std::stoi(ip_and_port.substr(ip_and_port.find(":") + 1, ip_and_port.size()));
    DataNode datanode(ip, port);
    datanode.start();
    return 0;
}
