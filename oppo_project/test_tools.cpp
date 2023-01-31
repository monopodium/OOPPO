#include<azure_lrc.h>

int main(){
    std::ofstream outfile;
    outfile.open("afile.dat");
    int *final_matrix;
    int k = 12;
    int g = 6;
    int l = 2;
    int *matrix = OppoProject::lrc_make_matrix(k, g, l, final_matrix);
    for (int i = 0; i < (l + g)*k;i++){
        outfile << matrix[i];
        outfile << ",";
    }
    return 0;
}