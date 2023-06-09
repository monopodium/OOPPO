#include <azure_lrc.h>

void OppoProject::dfs(std::vector<int> temp, std::shared_ptr<std::vector<std::vector<int>>> ans, int cur, int n, int k)
{
    if (int(temp.size()) + (n - cur + 1) < k)
    {
        return;
    }
    if (temp.size() == k)
    {
        ans->push_back(temp);
        return;
    }
    temp.push_back(cur);
    dfs(temp, ans, cur + 1, n, k);
    temp.pop_back();
    dfs(temp, ans, cur + 1, n, k);
}

bool OppoProject::combine(std::shared_ptr<std::vector<std::vector<int>>> ans, int n, int k)
{
    std::vector<int> temp;
    dfs(temp, ans, 1, n, k);
    return true;
}

bool OppoProject::check_k_data(std::vector<int> erasures, int k)
{
    int flag = 1;
    for (int i = 0; i < k; i++)
    {
        if (std::find(erasures.begin(), erasures.end(), i) != erasures.end())
        {
            flag = 0;
        }
    }
    if (flag)
    {
        return true;
    }

    return false;
}
bool OppoProject::lrc_make_matrix(int k, int g, int real_l, int *final_matrix)
{
    int r = (k + real_l - 1) / real_l;
    int *matrix = NULL;

    matrix = reed_sol_vandermonde_coding_matrix(k, g + 1, 8);
    if (matrix == NULL)
    {
        std::cout << "matrix == NULL" << std::endl;
    }

    // final_matrix = (int *)malloc(sizeof(int) * k * (g + real_l));
    if (final_matrix == NULL)
    {
        std::cout << "final_matrix == NULL" << std::endl;
    }
    bzero(final_matrix, sizeof(int) * k * (g + real_l));

    for (int i = 0; i < g; i++)
    {
        for (int j = 0; j < k; j++)
        {
            final_matrix[i * k + j] = matrix[(i + 1) * k + j];
        }
    }

    for (int i = 0; i < real_l; i++)
    {
        for (int j = 0; j < k; j++)
        {
            if (i * r <= j && j < (i + 1) * r)
            {
                final_matrix[(i + g) * k + j] = 1;
            }
        }
    }

    free(matrix);
    return true;
}



bool OppoProject::encode(int k, int m, int real_l, char **data_ptrs, char **coding_ptrs, int blocksize, EncodeType encode_type)
{
    int *matrix;
    matrix = reed_sol_vandermonde_coding_matrix(k, m, 8);
    if (encode_type == RS)
    {
        jerasure_matrix_encode(k, m, 8, matrix, data_ptrs, coding_ptrs, blocksize);
    }
    else if (encode_type == Azure_LRC_1)
    {
        std::vector<int> new_matrix((m + real_l) * k, 0);
        lrc_make_matrix(k, m, real_l, new_matrix.data());
        jerasure_matrix_encode(k, m + real_l, 8, new_matrix.data(), data_ptrs, coding_ptrs, blocksize);

        // 生成全局校验块的局部校验块
        std::vector<int> last_matrix(m, 1);
        jerasure_matrix_encode(m, 1, 8, last_matrix.data(), coding_ptrs, &coding_ptrs[m + real_l], blocksize);
    }
    else if (encode_type == OPPO_LRC)
    {
        int r = (k + real_l - 1) / real_l;
        jerasure_matrix_encode(k, m, 8, matrix, data_ptrs, coding_ptrs, blocksize);
        std::vector<int> az_g_number(real_l, 0);
        std::vector<int> az_data_number(real_l, 0);
        for (int i = 0; i < m; i++)
        {
            az_g_number[i % real_l] += 1;
        }
        for (int i = 0; i < real_l; i++)
        {
            az_data_number[i] = std::min(k - i * r, r);
        }
        int g_sum = 0;
        for (int i = 0; i < az_g_number.size(); i++)
        {
            std::vector<char *> vecotraz_g_number(az_data_number[i] + az_g_number[i]);
            char **new_data = (char **)vecotraz_g_number.data();

            for (int j = 0; j < az_data_number[i]; j++)
            {
                new_data[j] = data_ptrs[i * r + j];
            }
            for (int j = 0; j < az_g_number[i]; j++)
            {
                new_data[az_data_number[i] + j] = coding_ptrs[g_sum + j];
            }

            int shard_number_az = az_data_number[i] + az_g_number[i];
            std::vector<int> last_matrix(shard_number_az, 1);
            jerasure_matrix_encode(shard_number_az, 1, 8, last_matrix.data(), new_data, &coding_ptrs[m + i], blocksize);
            g_sum += az_g_number[i];
        }
    }
    free(matrix);
    return true;
}

bool OppoProject::decode(int k, int m, int real_l, char **data_ptrs, char **coding_ptrs, std::shared_ptr<std::vector<int>> erasures, int blocksize, EncodeType encode_type, bool repair)
{

    if (encode_type == RS || encode_type == OPPO_LRC)
    {
        std::vector<int> matrix(m * k, 0);
        int *rs_matrix = reed_sol_vandermonde_coding_matrix(k, m, 8);
        memcpy(matrix.data(), rs_matrix, m * k * sizeof(int));
        free(rs_matrix);
        if (jerasure_matrix_decode(k, m, 8, matrix.data(), 0, erasures->data(), data_ptrs, coding_ptrs, blocksize) != -1)
        {
            return true;
        }
    }
    else if (encode_type == Azure_LRC_1)
    {

        std::vector<int> matrix((m + real_l) * k, 0);
        lrc_make_matrix(k, m, real_l, matrix.data());
        if (!repair)
        {
            if (check_k_data(*erasures, k))
            {
                return true;
            }
        }
        if (jerasure_matrix_decode(k, m + real_l, 8, matrix.data(), 0, erasures->data(), data_ptrs, coding_ptrs, blocksize) == -1)
        {
            std::vector<int> new_erasures(m + real_l + 1, 1);
            int survival_number = k + m + real_l - erasures->size() + 1;
            std::vector<int> survival_index;
            auto part_new_erasure = std::make_shared<std::vector<std::vector<int>>>();
            for (int i = 0; i < int(erasures->size() - 1); i++)
            {
                new_erasures[i] = (*erasures)[i];
            }
            new_erasures[m + real_l] = -1;

            for (int i = 0; i < k + m + real_l; i++)
            {
                if (std::find(erasures->begin(), erasures->end(), i) == erasures->end())
                {
                    survival_index.push_back(i);
                }
            }
            if (survival_number > k)
            {
                combine(part_new_erasure, survival_index.size(), survival_number - k);
            }
            for (int i = 0; i < int(part_new_erasure->size()); i++)
            {
                for (int j = 0; j < int((*part_new_erasure)[i].size()); j++)
                {
                    new_erasures[erasures->size() - 1 + j] = survival_index[(*part_new_erasure)[i][j] - 1];
                }

                if (jerasure_matrix_decode(k, m + real_l, 8, matrix.data(), 0, new_erasures.data(), data_ptrs, coding_ptrs, blocksize) != -1)
                {
                    return true;
                    break;
                }
            }
        }
        else
        {
            return true;
        }
        std::cout << "cannot decode!!!!!!!!!!!!" << std::endl;
    }
    return false;
}

bool OppoProject::check_received_block(int k, int expect_block_number, std::shared_ptr<std::vector<int>> shards_idx_ptr, int shards_ptr_size)
{
    if (shards_ptr_size != -1)
    {
        if (int(shards_idx_ptr->size()) != shards_ptr_size)
        {
            return false;
        }
    }

    if (int(shards_idx_ptr->size()) >= expect_block_number)
    {
        return true;
    }
    else if (int(shards_idx_ptr->size()) == k) // azure_lrc防误杀
    {
        for (int i = 0; i < k; i++)
        {
            // 没找到
            if (std::find(shards_idx_ptr->begin(), shards_idx_ptr->end(), i) == shards_idx_ptr->end())
            {
                return false;
            }
        }
    }
    else
    {
        return false;
    }
    return true;
}
int OppoProject::check_decodable_azure_lrc(int k, int g, int l, std::vector<int> failed_block, std::vector<int> new_matrix)
{
    // 数据块，全局校验块，局部校验块
    // 检查是否满足理论可解
    std::vector<int> survive_block;
    for (int i = 0; i < k + l + g; i++)
    {
        if (std::find(failed_block.begin(), failed_block.end(), i) == failed_block.end())
        {
            survive_block.push_back(i);
        }
    }
    if (survive_block.size() != size_t(k))
    {
        return -2;
    }
    std::set<int> group_number;
    for (int block_index : survive_block)
    {
        group_number.insert(block_index / l);
    }
    if (survive_block.size() > g + group_number.size())
    {
        return -1;
    }

    std::vector<int> matrix((k + g + l) * k, 0);
    for (int i = 0; i < k; i++)
    {
        matrix[i * k + i] = 1;
    }
    for (int i = 0; i < (g + l) * k; i++)
    {
        matrix[k * k + i] = new_matrix[i];
    }
    std::vector<int> k_k_matrix(k * k, 0);

    for (size_t i = 0; i < survive_block.size(); i++)
    {
        for (int j = 0; j < k; j++)
        {
            k_k_matrix[i * k + j] = matrix[survive_block[i] * k + j];
        }
    }
    if (jerasure_invertible_matrix(k_k_matrix.data(), k, 8) == 0)
    {
        return -1;
    }
    return 1;
}

//wxh
bool OppoProject::calculate_data_delta(char* newdata,char* olddata,char* data_delta,int length){

    std::cout<<"old:"<<std::endl;
    std::cout<<std::string(olddata,length)<<std::endl;
    std::cout<<"new:"<<std::endl;
    std::cout<<std::string(newdata,length)<<std::endl;
    bzero(data_delta,length);
    std::vector<int> temp_mat(1*2,1);
    for(int i=0;i<temp_mat.size();i++) std::cout<<"cal data delta tempmat: "<<temp_mat[i]<<std::endl;
    std::vector<char*> data_ptrs;
    data_ptrs.push_back(newdata);
    data_ptrs.push_back(olddata);
    jerasure_matrix_encode(2,1,8,temp_mat.data(),data_ptrs.data(),&data_delta,length);
    std::cout<<"cal delta success"<<std::endl;
    std::cout<<"result: "<<std::endl<<std::string(data_delta,length)<<std::endl;
    std::vector<char> verified_new(length);
    bzero(verified_new.data(),length);
    memcpy(verified_new.data(),data_delta,length);
    galois_region_xor(olddata,verified_new.data(),length);
    std::cout<<"delta xor old:"<<std::endl;
    std::cout<<verified_new.data()<<std::endl;
    return true;
}



bool OppoProject::calculate_global_parity_delta(int k, int m, int real_l,char **data_ptrs, char **coding_ptrs, int blocksize,
                            std::vector<int> data_shard_idx,std::vector<int> parity_shard_idx, EncodeType encode_type){
    
    int *matrix;
    matrix=reed_sol_vandermonde_coding_matrix(k, m, 8);
    std::cout<<"cal global parity delta"<<std::endl;

    int sub_m=parity_shard_idx.size();
    int sub_k=data_shard_idx.size();//sub matrix row col

    std::cout<<"cal sub_m:"<<sub_m<<" sub_k:"<<sub_k<<std::endl;

    std::vector<int> temp_mat(sub_m*sub_k,0);
    

    std::cout<<"in cal global parity delta,data idxes :";
    /*
    for(int i=0;i<sub_k;i++){
        std::cout<<data_shard_idx[i]<<":";
    }
    */
    print_matrix(data_shard_idx.size(),1,0,data_shard_idx.data());

    std::cout<<std::endl;

    std::cout<<"in cal global parity delta,parity idxes :\n";
    print_matrix(parity_shard_idx.size(),1,0,parity_shard_idx.data());
    std::vector<int> primary_parity_idxes;//
    for(int i=0;i<sub_m;i++){
        primary_parity_idxes.push_back(parity_shard_idx[i]-k);//传入的global idx是+k之后的，在原始的范德蒙德矩阵中是第0，1...行   hard code 硬编码指定顺序
    }

    std::cout<<std::endl;
    if(encode_type==OppoProject::Azure_LRC_1)
    {
        std::vector<int> new_matrix((m + real_l) * k, 0);
        lrc_make_matrix(k, m, real_l, new_matrix.data());
        get_sub_matrix(k,m+real_l,new_matrix.data(),temp_mat.data(),data_shard_idx,primary_parity_idxes);
    } 
    else
    {
        get_sub_matrix(k,m,matrix,temp_mat.data(),data_shard_idx,primary_parity_idxes);
    } 
    std::cout<<"get submatrix returned ,submatirx is: "<<std::endl;
    /*
    for(int i=0;i<temp_mat.size();i++)
        std::cout<<" "<<temp_mat[i];
    */
    print_matrix(sub_k,sub_m,0,temp_mat.data());
    std::cout<<std::endl;
    jerasure_matrix_encode(sub_k,sub_m,8,temp_mat.data(),data_ptrs,coding_ptrs,blocksize);
    free(matrix);
    std::cout<<"cal global parity delta success"<<std::endl;
    return true;
}

bool OppoProject::calculate_local_parity_delta_azure_lrc1(int k,int m,int real_l,char **calculate_ptrs,char **coding_ptrs,int blocksize,
                                                std::vector<int> idxes,std::vector<int> local_idxes,bool local_of_global)
{
    /*
    @idxes 
    data or parity data delta->local delta, parity ->local delta
    @local_of_parity
    
    */
   int sub_k=idxes.size();
   int sub_l=local_idxes.size();
   int sub_m=local_idxes.size();
   if(local_of_global)
   {
        std::vector<int> temp_matrix(sub_k*sub_l,1);
        jerasure_matrix_encode(sub_k,sub_l,8,temp_matrix.data(),calculate_ptrs,coding_ptrs,blocksize);
   }
   else
    {
        std::vector<int> code_matrix((m + real_l) * k, 0);
        lrc_make_matrix(k, m, real_l, code_matrix.data());
        std::vector<int> temp_matrix(sub_l*sub_k,0);
        
        std::vector<int> primary_parity_idxes;//
        for(int i=0;i<sub_l;i++){
            std::cout<<local_idxes[i]<<":";
            primary_parity_idxes.push_back(local_idxes[i]-k);//传入的local idx是+k之后的，在原始的范德蒙德矩阵中是第m，m+1...行   hard encode 硬编码指定顺序
        }
        get_sub_matrix(k,m+real_l,code_matrix.data(),temp_matrix.data(),idxes,primary_parity_idxes);
        jerasure_matrix_encode(sub_k,sub_l,8,temp_matrix.data(),calculate_ptrs,coding_ptrs,blocksize);
    }
    return true;
}

bool OppoProject::calculate_local_parity_delta_oppo_lrc(int k,int m,int real_l,char **data_ptrs,char** global_ptrs,char **local_coding_ptrs,int blocksize,
                                               std::vector<int> data_idxes,std::vector<int> global_idxes,std::vector<int> local_idxes)
{
    //data delta XOR parity delta
    std::cout<<"cal local parity of oppo lrc"<<std::endl;
    int updated_shard_num=data_idxes.size()+global_idxes.size();
    std::vector<char *> new_data(updated_shard_num);
    char **all_cal_ptrs=(char**) new_data.data();
    int sub_k=data_idxes.size();
    for(int i=0;i<sub_k;i++)
        all_cal_ptrs[i]=data_ptrs[i];
    for(int j=0;j<global_idxes.size();j++)
        all_cal_ptrs[j+sub_k]=global_ptrs[j];
    
    std::vector<int> last_matrix(updated_shard_num, 1);
    jerasure_matrix_encode(updated_shard_num,1,8,last_matrix.data(),all_cal_ptrs,local_coding_ptrs,blocksize);
    
    return true;
}



bool OppoProject::get_sub_matrix(int k,int m,int *matrix,int *sub_matrix,std::vector<int> &data_idxes,std::vector<int> &parity_idxes)
{
    std::cout<<"origin matrix"<<std::endl;
    print_matrix(k,m,0,matrix);
    int sub_k=data_idxes.size();
    int sub_m=parity_idxes.size();

    std::cout<<"get submatrix k m sub_k sub_m :"<<k<<' '<<m<<' '<<sub_k<<' '<<sub_m<<' '<<std::endl;
    std::cout<<std::endl;
    for(int i=0;i<sub_m;i++)
    {
        for(int j=0;j<sub_k;j++)
        {
            
            sub_matrix[i*sub_k+j]=matrix[parity_idxes[i]*k+data_idxes[j]];
            
        }

    }

    std::cout<<"get submatrix over"<<std::endl;
    return true;
}


bool OppoProject::print_matrix(int k, int g, int real_l, int *matrix)
{   
    printf("%s","r  c ");//5个
    for(int i=0;i<k;i++)
    {
        printf("%5d",i);
    }
    std::cout<<std::endl;
    for(int i=0;i<g;i++)
    {
        printf("%5d",i+k);//校验块下标
        for(int j=0;j<k;j++)
            //std::cout<<matrix[i*k+j]<<' ';
            printf("%5d",matrix[i*k+j]);
        std::cout<<std::endl;
    }

    for(int i=0;i<real_l;i++)
    {
        //std::cout<<i+k+g;
        printf("%5d",i+k+g);
        for(int j=0;j<k;j++)
            //std::cout<<matrix[(i+g)*k+j]<<' ';
             printf("%5d",matrix[(i+g)*k+j]);
        std::cout<<std::endl;
    }
    return true;

}

/*默认更新所有的校验块，但是对于Azure_LRC_1，不是本AZ内的局部校验块是不需要更新的，对应的参数都是0，Jerasure会跳过这部分计算，只不过外面分配的的v_coding_area多分配了一块*/
bool OppoProject::RMW_encode(int k, int m, int real_l, char **update_data_ptrs, char **coding_ptrs, int blocksize, EncodeType encode_type,
                                std::vector<int> update_data_idxes)
{
    
    
    if(encode_type==RS)
    {
        std::vector<int> global_idxes;
        for(int i=k;i<k+m;i++) global_idxes.push_back(i);
        calculate_global_parity_delta(k,m,real_l,update_data_ptrs,coding_ptrs,blocksize,update_data_idxes,global_idxes,encode_type);
    }
    else if(encode_type == Azure_LRC_1) 
    {
        
        std::vector<int> global_local_idxes;
        for(int i=k;i<k+m+real_l;i++) global_local_idxes.push_back(i);//数据的local也当做global加入了，lrc_make_matrix之间数据是无关的
        calculate_global_parity_delta(k,m,real_l,update_data_ptrs,coding_ptrs,blocksize,update_data_idxes,global_local_idxes,encode_type);
       /*
        std::vector<int> global_idxes;
        for(int i=k;i<k+m;i++) global_idxes.push_back(i);
        calculate_global_parity_delta(k,m,real_l,update_data_ptrs,coding_ptrs,blocksize,update_data_idxes,global_idxes,encode_type);
        std::vector<int> local_idxes; 
        for(int i=k+m;i<k+m+real_l;i++) local_idxes.push_back(i);
        calculate_global_parity_delta(k,m,real_l,update_data_ptrs,&coding_ptrs[m],blocksize,update_data_idxes,local_idxes,encode_type);
        */

        std::vector<int> last_matrix(m, 1);
        
        jerasure_matrix_encode(m, 1, 8, last_matrix.data(), coding_ptrs, &coding_ptrs[m + real_l], blocksize);
    }
    else if (encode_type == OPPO_LRC)
    {
       
        std::vector<int> global_idxes;
        for(int i=k;i<k+m;i++) global_idxes.push_back(i);
        calculate_global_parity_delta(k,m,real_l,update_data_ptrs,coding_ptrs,blocksize,update_data_idxes,global_idxes,encode_type);

        //计算local
        int r = (k + real_l - 1) / real_l;
        std::vector<int> az_g_number(real_l, 0);
        std::vector<std::vector<int>> az_data_idx(real_l);
        std::vector<std::vector<char*>> az_data_ptrs(real_l);

        for (int i = 0; i < m; i++)
        {
            az_g_number[i % real_l] += 1;
        }
        for(int i=0;i<update_data_idxes.size();i++)
        {
            int idx=update_data_idxes[i];
            int az_id=idx/r;//易错
            az_data_idx[az_id].push_back(idx);
            az_data_ptrs[az_id].push_back(update_data_ptrs[i]);
        }
        

        int g_sum = 0;
        for(int i=0;i<az_g_number.size();i++)
        {
            std::vector<char *> data_global_ptrs(az_data_idx[i].size() + az_g_number[i]);
            char **new_data = (char **)data_global_ptrs.data();
            for(int j=0;j<az_data_ptrs[i].size();j++)
                new_data[j]=az_data_ptrs[i][j];
            for(int j=0;j<az_g_number[i];j++)
                new_data[az_data_ptrs[i].size() + j] = coding_ptrs[g_sum + j];
            
            int shard_number_az = data_global_ptrs.size();
            std::vector<int> last_matrix(shard_number_az, 1);
            jerasure_matrix_encode(shard_number_az, 1, 8, last_matrix.data(), new_data, &coding_ptrs[m + i], blocksize);
            g_sum += az_g_number[i];
        }

    }



    return true;
}

