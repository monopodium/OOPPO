#include <azure_lrc.h>
#include "toolbox.h"
#include <set>
namespace OppoProject
{
    bool partial_decoding_helper(std::unordered_map<int, std::vector<char>> all_origin, std::vector<int> failed_data_and_parity, std::unordered_map<int, std::unordered_map<int, std::vector<char>>> data_or_parity, int shard_size, int k, int g, int l)
    {
        std::set<int> func_idx;

        std::vector<int> matrix((g + l) * k, 0);
        lrc_make_matrix(k, g, l, matrix.data());
        // 首先判断能不能解码：
        std::vector<int> survive_block;
        for (int i = 0; i < k + l + g; i++)
        {
            if (std::find(failed_data_and_parity.begin(), failed_data_and_parity.end(), i) == failed_data_and_parity.end())
            {
                survive_block.push_back(i);
            }
        }
        int expect_funx_number = k;
        if (survive_block.size() <= size_t(k))
        {
            if (check_decodable_azure_lrc(k, g, l, failed_data_and_parity, matrix) != 1)
            {
                std::cout << "Not decodable!!!" << std::endl;
                return false;
            }
        }

        for (int i = 0; i < int(failed_data_and_parity.size()); i++)
        {
            if (failed_data_and_parity[i] >= k && failed_data_and_parity[i] < k + g + l)
            {
                func_idx.insert(failed_data_and_parity[i]);
            }
        }

        for (int i = k; i < (k + g + l); i++)
        {
            if (func_idx.size() == failed_data_and_parity.size())
            {
                break;
            }
            func_idx.insert(i);
        }
        std::cout << std::endl;

        for (int i = 0; i < g + l; i++)
        {
            for (int j = 0; j < k; j++)
            {
                std::cout << matrix[i * k + j] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "begining" << std::endl;
        for (auto &p : data_or_parity)
        {
            std::vector<std::pair<int, std::vector<char>>> saved_merge;
            std::vector<char> merge_result(shard_size);
            int count = p.second.size();
            std::vector<char *> v_data(count);
            std::vector<char *> v_coding(1);
            char **data = (char **)v_data.data();
            char **coding = (char **)v_coding.data();
            for (auto &t : func_idx)
            {
                std::vector<int> new_matrix(1 * count, 1);
                int real_idx = t - k;
                int *coff = &(matrix[real_idx * k]);
                // std::cout << "coff begin" << std::endl;
                // for (int i = 0; i < k; i++) {
                //     std::cout << coff[i] << " ";
                // }
                // std::cout << std::endl;
                // std::cout << "coff end" << std::endl;
                int idx = 0;
                for (auto &q : p.second)
                {
                    if (q.first < k)
                    {
                        new_matrix[idx] = coff[q.first];
                    }
                    else
                    {
                        new_matrix[idx] = (q.first == t);
                    }
                    data[idx] = q.second.data();
                    idx++;
                }
                coding[0] = merge_result.data();
                int sum = 0;
                for (auto &ss : new_matrix)
                {
                    sum += ss;
                    std::cout << ss << " ";
                }
                std::cout << std::endl;
                jerasure_matrix_encode(count, 1, 8, new_matrix.data(), data, coding, shard_size);
                if (sum == 0)
                {
                    std::vector<char> temp(shard_size, 0);
                    merge_result = temp;
                }
                saved_merge.push_back({t, merge_result});
            }
            p.second.clear();
            for (auto &q : saved_merge)
            {
                p.second[q.first] = q.second;
            }
        }
        std::cout << "ending" << std::endl;
        std::unordered_map<int, std::vector<char>> right;
        for (auto &p : func_idx)
        {
            int count = data_or_parity.size();
            std::vector<char *> v_data(count);
            std::vector<char *> v_coding(1);
            char **data = (char **)v_data.data();
            char **coding = (char **)v_coding.data();
            std::vector<char> temp(shard_size);
            int idx = 0;
            for (auto &q : data_or_parity)
            {
                data[idx++] = q.second[p].data();
            }
            coding[0] = temp.data();
            std::vector<int> new_matrix(1 * count, 1);
            jerasure_matrix_encode(count, 1, 8, new_matrix.data(), data, coding, shard_size);
            right[p] = temp;
        }
        std::vector<int> left;
        std::sort(failed_data_and_parity.begin(), failed_data_and_parity.end());
        std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@" << std::endl;
        for (auto &p : failed_data_and_parity)
        {
            std::cout << p << " ";
        }
        std::cout << std::endl;
        for (auto &p : func_idx)
        {
            int real_idx = p - k;
            int *coff = &(matrix[real_idx * k]);
            for (int i = 0; i < int(failed_data_and_parity.size()); i++)
            {
                if (failed_data_and_parity[i] < k)
                {
                    left.push_back(coff[failed_data_and_parity[i]]);
                }
                else
                {
                    left.push_back(failed_data_and_parity[i] == p);
                }
            }
        }
        std::cout << "======================================================" << std::endl;
        for (auto &p : left)
        {
            std::cout << p << " ";
        }
        std::cout << std::endl;
        std::vector<int> invert(left.size());
        jerasure_invert_matrix(left.data(), invert.data(), failed_data_and_parity.size(), 8);
        for (auto &p : invert)
        {
            std::cout << p << " ";
        }
        std::cout << std::endl;
        std::vector<char *> v_data(failed_data_and_parity.size());
        std::vector<char *> v_coding(failed_data_and_parity.size());
        char **data = (char **)v_data.data();
        char **coding = (char **)v_coding.data();
        std::vector<std::vector<char>> repaired_shards(failed_data_and_parity.size(), std::vector<char>(shard_size));
        int idx = 0;
        std::cout << "###############$$$$$$$$$$$$$$$$$$$$$" << std::endl;
        for (auto &p : func_idx)
        {
            std::cout << p << " ";
            data[idx] = right[p].data();
            coding[idx] = repaired_shards[idx].data();
            idx++;
        }
        std::cout << std::endl;
        jerasure_matrix_encode(failed_data_and_parity.size(), failed_data_and_parity.size(), 8, invert.data(), data, coding, shard_size);

        for (int i = 0; i < int(failed_data_and_parity.size()); i++)
        {
            int index = failed_data_and_parity[i];
            std::string origin(all_origin[index].data(), shard_size);
            std::string newshard(coding[i], shard_size);
            if (newshard == origin)
            {
                std::cout << i << " "
                          << "repair successfully!" << std::endl;
                // std::cout << origin << std::endl;
                // std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
                // std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
                // std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
                // std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
                // std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
                // std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
                // std::cout << newshard << std::endl;
            }
            else
            {
                std::cout << i << " "
                          << "repair fail!" << std::endl;
                std::cout << origin << std::endl;
                std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
                std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
                std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
                std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
                std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
                std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
                std::cout << newshard << std::endl;
            }
        }
    }
}
int main()
{
    int k = 12;
    int g = 6;
    int l = 2;
    std::vector<int> matrix;
    // int *rs_matrix = reed_sol_vandermonde_coding_matrix(k, g, 8);
    // matrix.resize(g * k);
    // memcpy(matrix.data(), rs_matrix, g * k * sizeof(int));
    // free(rs_matrix);

    // std::vector<int> failed_shard_idx1 = {1, 2, 6, 7, 11, 15, 18, 19};
    // std::cout << "check_decodable_azure_lrc1" << std::endl;
    // std::cout << OppoProject::check_decodable_azure_lrc(k, g, l, failed_shard_idx1) << std::endl;
    // std::cout << "check_decodable_azure_lrc2" << std::endl;
    // std::vector<int> failed_shard_idx2 = {1, 2, 5, 8, 11, 15, 16, 19};
    // std::cout << OppoProject::check_decodable_azure_lrc(k, g, l, failed_shard_idx2) << std::endl;

    int shard_size = 512;
    std::unordered_set<std::string> useless;
    std::string value = OppoProject::gen_key(shard_size * k, useless);

    int parity_blocks_number = g + l + 1;
    std::vector<char *> v_data(k);
    std::vector<char *> v_coding(parity_blocks_number);
    char **data = (char **)v_data.data();
    char **coding = (char **)v_coding.data();
    std::vector<std::vector<char>> coding_area(parity_blocks_number, std::vector<char>(shard_size));

    char *p_value = const_cast<char *>(value.data());
    for (int i = 0; i < k; i++)
    {
        data[i] = p_value + i * shard_size;
    }
    for (int i = 0; i < parity_blocks_number; i++)
    {
        coding[i] = coding_area[i].data();
    }
    OppoProject::encode(k, g, l, data, coding, shard_size, OppoProject::Azure_LRC_1);

    std::vector<int> failed_shard_idx = {2, 3, 6, 7, 14, 15, 18};
    std::unordered_map<int, std::unordered_map<int, std::vector<char>>> data_or_parity;
    int num_of_az = 3;
    int num_nodes_per_az = (k + g + l - failed_shard_idx.size()) / num_of_az;
    int idx = 0;
    for (int i = 0; i < num_of_az; i++)
    {
        for (int j = 0; j < num_nodes_per_az; j++)
        {
            while (failed_shard_idx.end() != std::find(failed_shard_idx.begin(), failed_shard_idx.end(), idx))
            {
                idx++;
            }
            std::cout << idx << std::endl;
            if (idx < k)
            {
                std::string temp(data[idx], shard_size);
                data_or_parity[i][idx++].assign(temp.begin(), temp.end());
            }
            else
            {
                std::string temp(coding[idx - k], shard_size);
                data_or_parity[i][idx++].assign(temp.begin(), temp.end());
            }
        }
    }
    std::unordered_map<int, std::vector<char>> all_origin;
    for (int i = 0; i < int(failed_shard_idx.size()); i++)
    {
        int index = failed_shard_idx[i];
        if (index < k)
        {
            std::string temp(data[index], shard_size);
            all_origin[index].assign(temp.begin(), temp.end());
        }
        else
        {
            std::string temp(coding[index - k], shard_size);
            all_origin[index].assign(temp.begin(), temp.end());
        }
    }
    OppoProject::partial_decoding_helper(all_origin, failed_shard_idx, data_or_parity, shard_size, k, g, l);
    return 0;
}