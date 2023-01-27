#include <azure_lrc.h>

int *OppoProject::lrc_make_matrix(int k, int g, int l, int *final_matrix)
{
    int r = (k + l - 1) / l;
    int *matrix = NULL;
    int *lrc_matrix = NULL;
    matrix = reed_sol_vandermonde_coding_matrix(k, g + 1, 8);
    if (matrix == NULL)
    {
        std::cout << "matrix == NULL" << std::endl;
    }

    lrc_matrix = (int *)malloc(sizeof(int) * k * (g + l));
    if (lrc_matrix == NULL)
    {
        std::cout << "lrc_matrix == NULL" << std::endl;
    }
    bzero(lrc_matrix, sizeof(int) * k * (g + l));

    for (int i = 0; i < l; i++)
    {
        for (int j = 0; j < k; j++)
        {
            if (i * r <= j && j < (i + 1) * r)
            {
                lrc_matrix[i * k + j] = 1;
            }
        }
    }
    for (int i = 0; i < g; i++)
    {
        for (int j = 0; j < k; j++)
        {
            lrc_matrix[(l + i) * k + j] = matrix[(i + 1) * k + j];
        }
    }
    free(matrix);
    return lrc_matrix;
}
