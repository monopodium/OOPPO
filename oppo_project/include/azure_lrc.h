#ifndef AZURELRC_H
#define AZURELRC_H
#include "jerasure.h"
#include "reed_sol.h"
#include "meta_definition.h"

namespace OppoProject
{
    bool lrc_make_matrix(int k, int g, int real_l, int *final_matrix);
    void dfs(std::vector<int> temp, std::shared_ptr<std::vector<std::vector<int>>> ans, int cur, int n, int k);
    bool combine(std::shared_ptr<std::vector<std::vector<int>>> ans, int n, int k);
    bool encode(int k, int m, int real_l, char **data_ptrs, char **coding_ptrs, int blocksize, EncodeType encode_type);
    bool decode(int k, int m, int real_l, char **data_ptrs, char **coding_ptrs, std::shared_ptr<std::vector<int>> erasures, int blocksize, EncodeType encode_type, bool repair = false);
    bool check_received_block(int k, int expect_block_number, std::shared_ptr<std::vector<int>> shards_idx_ptr, int shards_ptr_size = -1);
    bool check_k_data(std::vector<int> erasures, int k);
    int check_decodable_azure_lrc(int k, int g, int l, std::vector<int> failed_block, std::vector<int> new_matrix);
    bool calculate_data_delta(char* newdata,char* olddata,char* data_delta,int length);
    bool calculate_global_parity_delta(int k, int m, int real_l,char **data_ptrs, char **coding_ptrs, int blocksize,
                                       std::vector<int> data_shard_idx,std::vector<int> parity_shard_idx, EncodeType encode_type);
    bool calculate_local_parity_delta_azure_lrc1(int k,int m,int real_l,char **calculate_ptrs,char **coding_ptrs,int blocksize,
                                                std::vector<int> idxes,std::vector<int> local_idxes,bool local_of_global);//Azure LRC1
    bool calculate_local_parity_delta_oppo_lrc(int k,int m,int real_l,char **data_ptrs,char** global_ptrs,char **local_coding_ptrs,int blocksize,
                                               std::vector<int> data_idxes,std::vector<int> global_idxes,std::vector<int> local_idxes);//OPPO lrc
    //bool get_sub_matrix(int k,int m,int* matrix,int sub_k,int sub_m,int * sub_matrix,std::vector<int> row_idxes,std::vector<int> col_idxes);
    bool get_sub_matrix(int k,int m,int *matrix,int *sub_matrix,std::vector<int> &data_idxes,std::vector<int> &parity_idxes);

    bool print_matrix(int k, int g, int real_l, int *matrix);
}
#endif