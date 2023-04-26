#include "logmanager.h"


bool LogManager::append_to_log(OppoProject::LogEntry &log_entry)
{
    m_mutex.lock();
    log_file_ptr=fopen(m_log_file_path.c_str(),"a+b");
    if(log_file_ptr==NULL)
        std::cout<<"append to open error"<<'\n';
    fseek(log_file_ptr,0,SEEK_END);
    long long pos=ftell(log_file_ptr);
    std::cout<<log_entry.shard_id<<"  写入位置："<<pos<<std::endl;
    m_info_table[log_entry.shard_id].push_back(pos);
    
    for(int i=0;i<m_info_table[log_entry.shard_id].size();i++)
    {
        std::cout<<"pos: "<<m_info_table[log_entry.shard_id][i]<<std::endl;;
    }
    int shard_id_len=(log_entry.shard_id).length();

    fwrite(&shard_id_len,sizeof(shard_id_len),1,log_file_ptr);
    
    fwrite((log_entry.shard_id).c_str(),sizeof(char),shard_id_len,log_file_ptr);

    int offset=log_entry.offset_in_shard;
    int len=log_entry.length;
    fwrite(&offset,sizeof(int),1,log_file_ptr);
    fwrite(&len,sizeof(int),1,log_file_ptr);

    int delta_type=(int) log_entry.delta_type;
    fwrite(&delta_type,sizeof(int),1,log_file_ptr);

    fwrite((log_entry.delta_value_ptr).get()->data(),sizeof(char),len,log_file_ptr);
    fclose(log_file_ptr);
    m_mutex.unlock();
    /*
    std::cout<<"add entry success ,enty is: "<<std::endl;
    std::cout<<log_entry.shard_id<<" "<<offset<<" "<<len<<" "<<log_entry.delta_value_ptr.get()->data()<<std::endl;
    std::cout<<"add entry finished \n"<<std::endl;
    */

    return true;
}

bool LogManager::find_delta(std::string shard_id)
{
    int num=0;
    m_mutex.lock();
    num=m_info_table[shard_id].size();
    m_mutex.unlock();
    if(num>0) return true;
    else return false;
    
}


bool LogManager::get_entries(std::string shard_id,std::vector<OppoProject::LogEntry> &log_entries)
{
    m_mutex.lock();
    log_file_ptr=log_file_ptr=fopen(m_log_file_path.c_str(),"rb");
    if(log_file_ptr==NULL)
    {
        std::cout<<"open log file error\n";
        return false;
    }
    std::cout<<"open log file success\n";
        


    auto pos_vec=m_info_table[shard_id];
    std::cout<<"pos vec: \n";

    for(int i=0;i<pos_vec.size();i++)
        std::cout<<pos_vec[i]<<" : ";
    std::cout<<std::endl;

    for(int i=0;i<pos_vec.size();i++)
    {   
        OppoProject::LogEntry log_entry;
        long long t_pos=pos_vec[i];
        fseek(log_file_ptr,t_pos,SEEK_SET);

        int shard_id_len;
        fread(&shard_id_len,sizeof(int),1,log_file_ptr);

        std::vector<char> t_shardid(shard_id_len);
        fread(t_shardid.data(),sizeof(char),shard_id_len,log_file_ptr);

        std::string shard_id(t_shardid.begin(),t_shardid.end());

        int offset;
        int len;
        fread(&offset,sizeof(int),1,log_file_ptr);
        fread(&len,sizeof(int),1,log_file_ptr);
        
        std::cout<<"offset:"<<offset<<" len: "<<len<<std::endl;

        int t_type;
        fread(&t_type,sizeof(int),1,log_file_ptr);
        
        OppoProject::DeltaType delta_type=(OppoProject::DeltaType) t_type;

        auto value_ptr=std::make_shared<std::vector<unsigned char>>(len);
        
        fread(value_ptr.get()->data(),sizeof(char),len,log_file_ptr);

        log_entries.push_back(OppoProject::LogEntry(shard_id,offset,len,delta_type,value_ptr));
        
        std::cout<<"get log_entry, shardid:"<<shard_id<<" offset:"<<offset<<" len:"<<len<<" delta value: "<<std::endl;
        std::cout<<std::string((char*)value_ptr->data(),len)<<std::endl;;
        std::cout<<std::endl;
    }

    fclose(log_file_ptr);
    m_mutex.unlock();

    return true;

}

bool LogManager::merge_with_parity(std::string shard_id,char *parity_ptr, int parity_len)
{

    std::vector<OppoProject::LogEntry> log_entries;
    get_entries(shard_id,log_entries);

    for(int i=0;i<log_entries.size();i++)
    {
        int delta_offset=log_entries[i].offset_in_shard;
        int delta_len=log_entries[i].length;
        if(delta_offset+delta_len>parity_len)
        {
            std::cerr<<"merge delta ,but to long delta,fail"<<std::endl;
            return false;
        } 
        
        std::vector<char> temp_result(delta_len);
        OppoProject::calculate_data_delta(parity_ptr+delta_offset,(char*)(log_entries[i].delta_value_ptr.get())->data(),temp_result.data(),delta_len);
        memcpy(parity_ptr+delta_offset,temp_result.data(),delta_len);
    }

    return true;
}

bool LogManager::init(std::string log_file_path)
{
    this->m_log_file_path=log_file_path;
    log_file_ptr=fopen(m_log_file_path.c_str(),"wb");
    if(log_file_ptr==NULL) std::cout<<"create file failed"<<std::endl;
    fclose(log_file_ptr);
}

