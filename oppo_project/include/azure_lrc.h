#ifndef AZURELRC_H
#define AZURELRC_H
#include "jerasure.h"
#include "reed_sol.h"
#include <iostream>
#include <string.h>
#include <fstream>
namespace OppoProject
{
    int *lrc_make_matrix(int k, int g, int l, int *final_matrix);
}
#endif