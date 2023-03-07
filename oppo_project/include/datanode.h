#ifndef DATANODE_H
#define DATANODE_H

#include <libmemcached/memcached.h>
#include <asio.hpp>
#include <string>

class DataNode
{
public:
    DataNode(std::string ip, int port) : ip(ip), port(port),
                                         acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::address::from_string(ip.c_str()), port))
    {
        memcached_return rc;
        m_memcached = memcached_create(NULL);
        memcached_server_st *servers;
        // libmemcached和memcached在同一个节点上，端口号相差1000
        servers = memcached_server_list_append(NULL, ip.c_str(), port - 1000, &rc);
        rc = memcached_server_push(m_memcached, servers);
        memcached_server_free(servers);
        memcached_behavior_set(m_memcached, MEMCACHED_BEHAVIOR_DISTRIBUTION, MEMCACHED_DISTRIBUTION_CONSISTENT);
        memcached_behavior_set(m_memcached, MEMCACHED_BEHAVIOR_RETRY_TIMEOUT, 20);
        memcached_behavior_set(m_memcached, MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT, 5);
        memcached_behavior_set(m_memcached, MEMCACHED_BEHAVIOR_AUTO_EJECT_HOSTS, true);
    }
    void start();

private:
    void do_work();
    memcached_st *m_memcached;
    std::string ip;
    int port;
    asio::io_context io_context;
    asio::ip::tcp::acceptor acceptor;
};

#endif