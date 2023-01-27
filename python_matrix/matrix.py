# Linear Algebra Learning Sequence
# Vandermonde Matrix
 
import numpy as np
import math
import itertools
from itertools import product

def multi_product(A):
    last_list = []
    for i in range(1,len(A)):
        if i==1:
            last_list = A[len(A)-1]
        last_list = product(A[len(A)-i-1],last_list)
        last_list_new = []
        for x in list(last_list):
            new_item = []
            for item in list(x):
                new_item += item
            last_list_new.append(new_item)
        last_list= last_list_new
        # if i > 1: 
        #     for i in range(len(last_list)):
        #         temp_list = [last_list[i][0]]+list(last_list[i][1])
        #         last_list[i] = tuple(temp_list)     
    return last_list


def init_matrix(k,l,g):

    r = math.ceil(k/l)
    #g = l
    matrix_k = np.eye(k)
    matrix_l_origin = []
    for i in range(l):
        matrix_l_origin.append([])
        for j in range(k):
            if i * r <= j < (i+1)*r:
                matrix_l_origin[i].append(1)
            else:
                matrix_l_origin[i].append(0)
            
    matrix_l = np.array(matrix_l_origin)


    x = np.array(range(1,g+2))
    N = k
    matrix_g = np.vander(x,N,True)
    matrix_full = np.vstack((np.vstack((matrix_k,matrix_l)),matrix_g[1:,]))
    return matrix_full
def if_Target_matrix(k,l,g,matrix_full):
    r = math.ceil(k/l)
    azure_lrc_flag = 1
    for i in range(l+1):
    #The failures are equally divided between group x and group y.
        failures = g + l - i
        non_failures_list = []
        max_failures = math.ceil(failures/l)
        #假设每一组都有r个数据块吧呜呜呜
        for j in range(l):
            non_failures_list.append(r - min(max_failures,failures-max_failures*j))
        list_to_combine = []
        for ij in range(l):
            list_to_combine1 = [list(item) for item in list(itertools.combinations(list(range(ij*r,ij*r+r)),non_failures_list[ij]))]
            list_to_combine.append(list_to_combine1)
        
        list_to_combine1 = [list(item) for item in itertools.combinations(list(range(l*r,l*r+l)),l-i)]
        list_to_combine.append(list_to_combine1)

        non_failures_index = multi_product(list_to_combine)
        
        for item in non_failures_index:
            item1 = item + list(range(k+l,k+g+l))
            if(np.linalg.det(matrix_full[item1])==0):
                azure_lrc_flag = 0
    if azure_lrc_flag == 1:
        print("Gongradulations,this matrix satisfies the requirement!")
    else:
        print("bad matrix!")
    return azure_lrc_flag
if __name__ == "__main__":
    k = 12
    l = 2
    g = 6
    matrix = init_matrix(k,l,g)
    if_Target_matrix(k,l,g,matrix)