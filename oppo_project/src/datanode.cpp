#include "datanode.h"
#include "toolbox.h"
#include "meta_definition.h"

void DataNode::do_work()
{
    for (;;)
    {
        asio::ip::tcp::socket socket(io_context);
        acceptor.accept(socket);
        std::vector<unsigned char> int_buf_flag(sizeof(int));
        asio::read(socket, asio::buffer(int_buf_flag, int_buf_flag.size()));
        int flag = OppoProject::bytes_to_int(int_buf_flag);
        if (flag == 0)
        {
            try
            {
                std::vector<unsigned char> int_buf(sizeof(int));
                asio::read(socket, asio::buffer(int_buf, int_buf.size()));
                int key_size = OppoProject::bytes_to_int(int_buf);
                asio::read(socket, asio::buffer(int_buf, int_buf.size()));
                int value_size = OppoProject::bytes_to_int(int_buf);

                std::vector<unsigned char> key_buf(key_size);
                std::vector<unsigned char> value_buf(value_size);
                asio::read(socket, asio::buffer(key_buf, key_buf.size()));
                asio::read(socket, asio::buffer(value_buf, value_buf.size()));

                memcached_return_t ret = memcached_set(m_memcached, (const char *)key_buf.data(), key_buf.size(), (const char *)value_buf.data(), value_buf.size(), (time_t)0, (uint32_t)0);
                if (memcached_failed(ret))
                {
                    std::cout << "memcached_set fail" << std::endl;
                    printf("%d %s %d %d\n", key_size, std::string((char *)key_buf.data(), key_size).c_str(), port, ret);
                }
                std::vector<char> finish(1);
                asio::write(socket, asio::buffer(finish, finish.size()));
                asio::error_code ignore_ec;
                socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
                socket.close(ignore_ec);
            }
            catch (std::exception &e)
            {
                std::cout << e.what() << std::endl;
                exit(-1);
            }
        }
        else if (flag == 1)
        {
            try
            {
                std::vector<unsigned char> int_buf(sizeof(int));
                asio::read(socket, asio::buffer(int_buf, int_buf.size()));
                int key_size = OppoProject::bytes_to_int(int_buf);

                std::vector<unsigned char> key_buf(key_size);
                asio::read(socket, asio::buffer(key_buf, key_buf.size()));

                asio::read(socket, asio::buffer(int_buf, int_buf.size()));
                int offset = OppoProject::bytes_to_int(int_buf);

                asio::read(socket, asio::buffer(int_buf, int_buf.size()));
                int lenth = OppoProject::bytes_to_int(int_buf);
                
                
                memcached_return_t error;
                uint32_t flag;
                size_t value_size;
                char *value_ptr = memcached_get(m_memcached, (const char *)key_buf.data(), key_buf.size(), &value_size, &flag, &error);
                if (value_ptr == NULL)
                {
                    std::cout << "memcached_get fail" << std::endl;
                    printf("%d %s %d %d\n", key_size, std::string((char *)key_buf.data(), key_size).c_str(), port, error);
                }
                
                
                //wxh
                logmanager.merge_with_parity(std::string(key_buf.begin(),key_buf.end()),value_ptr,value_size);


                asio::write(socket, asio::buffer(value_ptr + offset, lenth));
                asio::error_code ignore_ec;
                socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
                socket.close(ignore_ec);
            }
            catch (std::exception &e)
            {
                std::cout << e.what() << std::endl;
                exit(-1);
            }
        }
        else if(flag==2)
        {//receive delta ,and write into log
            try
            {
                asio::error_code error;
                int key_size = OppoProject::receive_int(socket,error);
                int update_data_size=OppoProject::receive_int(socket,error);

                std::vector<unsigned char> key_buf(key_size);
                
                std::cout<<"update len:"<<update_data_size<<std::endl;

                std::shared_ptr<std::vector<unsigned char> > update_data_ptr=std::make_shared<std::vector<unsigned char>>(update_data_size);

                asio::read(socket, asio::buffer(key_buf, key_buf.size()));
                std::cout<<"receive key: "<<std::string((char *)key_buf.data(), key_size)<<"len: "<<update_data_size<<std::endl;

                asio::read(socket, asio::buffer(update_data_ptr.get()->data(), update_data_size));
                std::cout<<"receive delta is: "<<std::string((char*) update_data_ptr.get()->data())<<std::endl;
                
                int offset_in_shard=OppoProject::receive_int(socket,error);
                int delta_type=OppoProject::receive_int(socket,error);
                std::cout<<"receive delta ,delta type:"<<delta_type<<std::endl;
                
                
                
            
                std::vector<char> finish(1);
                asio::write(socket, asio::buffer(finish, finish.size()));
                asio::error_code ignore_ec;
                socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
                socket.close(ignore_ec);

                std::cout<<"update shardid:"<<key_buf.data()<<" offset"<<offset_in_shard<<std::endl;

                OppoProject::LogEntry log_entry(std::string(key_buf.begin(),key_buf.end()),offset_in_shard,update_data_size,(OppoProject::DeltaType)delta_type,update_data_ptr);
                logmanager.append_to_log(log_entry);
            }
            catch (std::exception &e)
            {
                std::cout << e.what() << std::endl;
                exit(-1);
            }
            
        }
    }
}

void DataNode::start()
{
    std::thread work(std::bind(&DataNode::do_work, this));
    work.join();
}