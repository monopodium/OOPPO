#ifndef RUN_RMW_H
#define RUN_RMW_H

#include "jerasure.h"


bool RMW_test(int k,int m,std::string olddata,std::string newdata,int offset,int length,int blocksize,std::vector<int> data_idxes,std::vector<int> parity_idxes);


#endif