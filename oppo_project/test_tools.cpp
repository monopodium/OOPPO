#include <azure_lrc.h>

int main()
{
    std::ofstream outfile;
    outfile.open("afile.dat");
    int *final_matrix;
    int k = 12;
    int g = 6;
    int l = 2;
    auto part_new_erasure = std::make_shared<std::vector<std::vector<int>>>();

    std::vector<int> survival_index;
    // for (int i = 0; i < k + g + l; i++)
    // {

    //     survival_index.push_back(i);
    // }
    OppoProject::combine(part_new_erasure, 10, 3);
    for (int i = 0; i < part_new_erasure->size();i++){
        std::cout << (*part_new_erasure)[i][0] << std::endl;
    }
        final_matrix = (int *)malloc(sizeof(int) * k * (g + l));
    OppoProject::lrc_make_matrix(k, g, l, final_matrix);
    for (int i = 0; i < (l + g) * k; i++)
    {
        outfile << final_matrix[i];
        outfile << ",";
    }
    return 0;
}