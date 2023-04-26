#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <mutex>
#include <unordered_map>
#include <vector>
#include "meta_definition.h"
#include "azure_lrc.h"



class LogManager
{
private:
    /* data */
    std::mutex m_mutex;
    std::string m_log_file_path;
    std::unordered_map<std::string,std::vector<long long> > m_info_table;//shardid->offset_in_file
    FILE* log_file_ptr;

public:
    LogManager()
    {
        
    };
    ~LogManager()
    {
        if(log_file_ptr!=NULL)
    {
        fclose(log_file_ptr);
    }
        
    };
    
    bool append_to_log(OppoProject::LogEntry &log_entry);
    bool find_delta(std::string shard_id);
    bool get_entries(std::string shard_id,std::vector<OppoProject::LogEntry> &log_entries);
    bool merge_with_parity(std::string shard_id,char *parity_ptr, int parity_len);
    bool init(std::string log_file_path);
};



#endif
