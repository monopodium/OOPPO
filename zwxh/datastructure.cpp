#include<iostream>
/*data proxy*/


std::unordered_map<int,OppoProject::ShardidxRange> data_idx_ranges; //idx->(idx,offset,length) 
std::unordered_map<int,std::pair<std::string, int>> data_nodes_ip_port;//idx->node ip port
std::vector<int> localparity_idxes;
std::vector<int> globalparity_idxes;  
std::unordered_map<int,std::vector<char> > new_shard_data_map;//idx->data
std::unordered_map<int,std::vector<char> > old_shard_data_map;
std::unordered_map<int,std::vector<char> > cur_az_data_delta_map;

std::map<int,std::vector<char>> received_idx_delta;


std::vector<std::vector<char> > cur_az_global_deltas;
std::vector<char *> cur_global_parity_ptrs;
std::vector<char> local_parity_delta(blocksize);
std::vector<char *> local_ptrs;
local_ptrs.push_back(local_parity_delta.data());

std::vector<char*> all_update_data_delta_ptrs; //没有分配空间 没有直接初始化分配空间
std::vector<int> all_update_data_idx;

/*collector proxy*/
std::unordered_map<int,OppoProject::ShardidxRange> data_idx_ranges; //idx->(idx,offset,length) 
std::unordered_map<int,std::pair<std::string, int>> data_nodes_ip_port;//idx->node ip port
std::vector<int> localparity_idxes;
std::vector<int> globalparity_idxes;

std::unordered_map<int,std::vector<char> > new_shard_data_map;//idx->data
std::unordered_map<int,std::vector<char> > old_shard_data_map;
std::unordered_map<int,std::vector<char> > cur_az_data_delta_map;//包括从proxy 接收以及本AZ计算得到的
 //跨AZ传输parity delta
std::vector<OppoProject::ParityDeltaSendType> v_parity_delta_send_type;
std::vector<int> cross_AZ_parityshard_idxes;
std::unordered_map<int,std::pair<std::string, int>> cross_AZ_parityshard_nodes_ip_port;//上面位于其他AZ global parity idx->node ip port
//从data proxy收delta需要的结构     
std::map<int,std::vector<char>> all_idx_data_delta;//存delta idx->delta  
//std::unordered_map<std::pair<std::string,int>, std::vector<int> > datadelta_idx_dataproxy_had;//记录data proxy有的delta ，防止data delta更新时跨AZ发送冗余
int offset_from_data_proxy=0;
int length_from_data_proxy=0;
OppoProject::DeltaType delta_type_from_proxy;

std::vector<int> all_update_data_idx;
all_update_data_idx.clear();
//std::vector<char *> all_update_data_delta_ptrs;
std::vector<char *> all_update_data_delta_ptrs;
all_update_data_delta_ptrs.clear();
std::vector<std::vector<char> > all_delta_vector;

std::cout<<"7. 跨AZ更新"<<std::endl;
std::vector<int> cross_az_parity_idxes;
std::vector<std::vector<char> > cross_az_parity_deltas;
std::vector<char *> parity_ptrs;

//本AZ更新

std::cout<<"start  update self"<<std::endl;
std::vector<std::vector<char> > cur_az_global_deltas;
std::vector<char *> cur_global_parity_ptrs;
std::vector<char> local_parity_delta(blocksize);
std::vector<char *> local_ptrs;
local_ptrs.push_back(local_parity_delta.data());

std::vector<char *> cur_az_data_delta_ptrs;
std::vector<int> cur_az_data_delta_idxes;