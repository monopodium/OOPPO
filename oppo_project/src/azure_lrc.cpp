#include <azure_lrc.h>

bool OppoProject::lrc_make_matrix(int k, int g, int l, int *final_matrix)
{
    int r = (k + l - 1) / l;
    int *matrix = NULL;

    matrix = reed_sol_vandermonde_coding_matrix(k, g + 1, 8);
    if (matrix == NULL)
    {
        std::cout << "matrix == NULL" << std::endl;
    }

    // final_matrix = (int *)malloc(sizeof(int) * k * (g + l));
    if (final_matrix == NULL)
    {
        std::cout << "final_matrix == NULL" << std::endl;
    }
    bzero(final_matrix, sizeof(int) * k * (g + l));

    for (int i = 0; i < g; i++)
    {
        for (int j = 0; j < k; j++)
        {
            final_matrix[i * k + j] = matrix[(i + 1) * k + j];
        }
    }

    for (int i = 0; i < l; i++)
    {
        for (int j = 0; j < k; j++)
        {
            if (i * r <= j && j < (i + 1) * r)
            {
                final_matrix[(i + g) * k + j] = 1;
            }
        }
    }
    std::cout << "final_matrix" << std::endl;
    for (int i = 0; i < g + l; i++)
    {
        for (int j = 0; j < k; j++)
        {
            std::cout << final_matrix[i * k + j] << ",";
        }
        std::cout << std::endl;
    }
    // free(matrix);
    return true;
}

bool OppoProject::encode(int k, int m, int l, char **data_ptrs, char **coding_ptrs, int blocksize, EncodeType encode_type)
{
    int *matrix;
    matrix = reed_sol_vandermonde_coding_matrix(k, m, 8);
    if (encode_type == RS)
    {
        jerasure_matrix_encode(k, m, 8, matrix, data_ptrs, coding_ptrs, blocksize);
    }
    else if (encode_type == Azure_LRC_1 || encode_type == OPPO_LRC)
    {
        std::vector<int> new_matrix((m + l) * k, 0);
        std::cout << "new_matrix((m + l) * k, 0)" << std::endl;
        if (encode_type == Azure_LRC_1)
        {
            lrc_make_matrix(k, m, l, new_matrix.data());
        }
        else
        {
            memcpy(new_matrix.data(), matrix, m * k * sizeof(int));
        }
        jerasure_matrix_encode(k, m + l, 8, new_matrix.data(), data_ptrs, coding_ptrs, blocksize);

        std::cout << "encode=========================================" << std::endl;
        for (int i = 0; i < k; i++)
        {
            std::cout << "i" << i << std::endl;
            std::cout << std::string(data_ptrs[i], blocksize) << std::endl;
        }
        for (int i = 0; i < m + l; i++)
        {
            std::cout << "i" << i << std::endl;
            std::cout << std::string(coding_ptrs[i], blocksize) << std::endl;
        }
        std::cout << "encode=========================================" << std::endl;

        // 生成全局校验块的局部校验块
        std::vector<char> last_local(blocksize, 0);
        std::vector<char *> v_temp_coding(1);
        char **temp_coding = v_temp_coding.data();
        temp_coding[0] = last_local.data();
        std::vector<int> last_matrix(m, 1);
        jerasure_matrix_encode(m, 1, 8, last_matrix.data(), coding_ptrs, temp_coding, blocksize);
        memcpy(coding_ptrs[m + l], last_local.data(), blocksize);
    }
    return true;
}

bool OppoProject::decode(int k, int m, int l, char **data_ptrs, char **coding_ptrs, std::shared_ptr<std::vector<int>> erasures, int blocksize, EncodeType encode_type)
{

    if (encode_type == RS || encode_type == OPPO_LRC)
    {
        std::vector<int> matrix(m * k, 0);
        int *rs_matrix = reed_sol_vandermonde_coding_matrix(k, m, 8);
        memcpy(matrix.data(), rs_matrix, m * k * sizeof(int));
        jerasure_matrix_decode(k, m, 8, matrix.data(), 0, erasures->data(), data_ptrs, coding_ptrs, blocksize);
    }
    else if (encode_type == Azure_LRC_1)
    {
        std::vector<int> matrix((m + l) * k, 0);
        std::cout << "lrc_make_matrix" << std::endl;
        lrc_make_matrix(k, m, l, matrix.data());
        std::cout << "decode=========================================" << std::endl;
        for (int i = 0; i < k; i++)
        {
            std::cout << "i" << i << std::endl;
            std::cout << std::string(data_ptrs[i], blocksize) << std::endl;
        }
        for (int i = 0; i < m + l; i++)
        {
            std::cout << "i" << i << std::endl;
            std::cout << std::string(coding_ptrs[i], blocksize) << std::endl;
        }
        std::cout << "decode=========================================" << std::endl;

        int flag = 1;
        // 如果找到了
        for (int i = 0; i < k; i++)
        {
            if (std::find(erasures->begin(), erasures->end(), i) != erasures->end())
            {
                flag = 0;
            }
        }
        if (flag)
        {
            return true;
        }
        else
        {
            
            std::vector<int> new_erasures(m+l+1,1);
            for (int i = 0; i < erasures->size()-1; i++)
            {
                new_erasures[i] = (*erasures)[i];
            }
            for (int i = 0; i < k + m + l; i++)
            {

                // if (std::find(erasures->begin(), erasures->end(), i) == erasures->end())
                // {

                    new_erasures[m+l-1] = i;
                    new_erasures[m+l] = -1;
                    for (int i = 0; i < new_erasures.size(); i++)
                    {
                        std::cout << "erasures.size():" << new_erasures[i] << std::endl;
                    }
                    if (jerasure_matrix_decode(k, m, 8, matrix.data(), 0, new_erasures.data(), data_ptrs, coding_ptrs, blocksize) == -1)
                    {

                        std::cout << "cannot decode" << std::endl;
                    }
                    else
                    {
                        return true;
                    }
                // }
            }
        }
    }
    std::cout << "jerasure_matrix_decode" << std::endl;
    std::cout << "cannot decode!!!!!!!!!!!!" << std::endl;
    return true;
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
