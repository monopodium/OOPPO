# Linear Algebra Learning Sequence
# Vandermonde Matrix
 
import numpy as np
import math
import itertools
import galois
from itertools import product
from tqdm import tqdm

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
    return last_list


def init_matrix(k,l,g):

    r = math.ceil(k/l)
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

    GF = galois.GF(2**8)
    x = np.array(range(2,k+3))
    x = x.astype(np.int32)
    

    N = g + 1
    a = GF.primitive_element
    V = GF.Vandermonde(GF([[17,12,3,4],[17,12,5,6]])._primitive_element , 4, 4)
    #xx = galois.Array.Random((1,4),GF(0),GF(2),seed = 2,dtype=int)
    print(V)
    matrix_g = np.vander(x,N,True)
    matrix_g = matrix_g.transpose()  
    matrix_full = np.vstack((np.vstack((matrix_k,matrix_l)),matrix_g[1:g + 1,1:]))
    matrix_full = matrix_full.astype(np.int32)
    print(matrix_full)
    
    return matrix_full

def init_matrix_by_file(file_name):

    r = math.ceil(k/l)
    #g = l
    matrix_k = np.eye(k)
    matrix_l_g = []
    with open(file_name) as f:
        for line in f.readlines():
            list_line = line.split(",")
            for i in range(l+g):
                small_matrix = []
                for j in range(k):
                    small_matrix.append(int(list_line[k*i+j]))
                matrix_l_g.append(small_matrix)
                

    
    matrix_full = np.vstack((matrix_k,matrix_l_g))
    print(matrix_full)
    return matrix_full
    
#每个组中数据块坏的数量为[0，r],是否满足“没有一个组的块全坏”的条件在后面判断，不在此处限制。
def find_all(limit_list,n):
    '''
    返回x1+x2+x3..=n的所有满足要求的解
    '''
    all_list = []
    for limit in limit_list:
        item = []
        for i in range(limit+1):
            item.append(i)
        all_list.append(item)
    all_solution = []
    for i in range(1,len(all_list)):
        if i==1:
            all_solution = all_list[len(all_list)-1]
        all_solution =  product(all_list[len(all_list)-i-1], all_solution)
        if i==1:
            all_solution = [list(item) for item in list(all_solution)]    
        else:
            all_solution = [[item[0]]+item[1] for item in list(all_solution)]
    final_solution = []
    for item in all_solution:
        if sum(item) == n:
            final_solution.append(item)
    return final_solution
            
                
def check_info_theo(k,l,g,r,item):
    failures = []
    for i in range(k+l+g):
        if i not in item:
            failures.append(i)
    
    group_set = set()
    for i in failures:
        if i < k:
            group_set.add(math.floor(i/r))
        if k <= i and i < k + l:
            group_set.add(i - k)
    if len(failures) <= g + len(group_set):
        # print("True failures",failures)
        # print(item)
        return True
    # print("False failures",failures)
    # print(item)
    return False

  
    
def if_Target_matrix_final(k,l,g,matrix_full):
    r = math.ceil(k/l)
    n = k + l + g
    azure_lrc_flag = 1
    
    GF = galois.GF(2**8)
    matrix_full = matrix_full.astype(np.int32)
    matrix_full = matrix_full.view(GF)
    #i为错误的数量，
    for i in range(g+l,g+1,-1):
        failures_number = i
        survival_number = k + l + g - failures_number
        list_to_combine1 = [list(item) for item in list(itertools.combinations(list(range(0,n)),survival_number))]

        for item in reversed(list_to_combine1):
            if i == g+l:
                if check_info_theo(k,l,g,r,item):
                    if(np.linalg.det(matrix_full[item])==0):
                        azure_lrc_flag = 0
                
            else:
                list_to_combine2 = [list(each_item) for each_item in list(itertools.combinations(item,n-g-l))]
                small_flag = 0
                for sub_matrix in list_to_combine2:
                    if check_info_theo(k,l,g,r,item):
                        if(np.linalg.det(matrix_full[item])!=0):
                            small_flag = 1
                if small_flag==0:
                    azure_lrc_flag = 0
                    break
            if azure_lrc_flag==0:
                # print(item)
                # print("bad matrix!")
                break
    
    if azure_lrc_flag == 1:
        print(matrix_full)
        print("Gongradulations,this matrix satisfies the requirement!")
    else:
        print("bad matrix!")
    return azure_lrc_flag            

def test_gf():
    GF = galois.GF(2**8)
    y = np.array([[109, 17, 108, 224],
                 [19, 17, 18, 224],
                 [10, 1, 8, 4],
                 [9, 17, 1, 2]]
                 )
    y = y.view(GF)
    print("y",y)
    print("np.linalg.det(y)")
    print(np.linalg.det(y))
    print("np.linalg.det(np.array([109, 17, 108, 224]))")
    print(np.linalg.det([[109, 17, 108, 224],
                 [19, 17, 18, 224],
                 [10, 1, 8, 4],
                 [9, 17, 1, 2]]))   
    
def search_matrix(k,l,g):
    r = math.ceil(k/l)
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
    GF = galois.GF(2**4)
    seed1 = 0
    while(1):
        # x = np.array(range(2,k+2))
        # x = x.view(GF)
        seed1 = seed1 + 1
        x = GF.Random(k, low=0, high=15, seed=seed1)
        matrix_g = []
        x_n = np.array([1]*k)
        x_n = x_n.view(GF)
        for i in range(g):
            x_n = x_n*x
            matrix_g.append(x_n)
        matrix_g = np.array(matrix_g).view(GF)
        matrix_full = np.vstack((np.vstack((matrix_k,matrix_l)),matrix_g))
        matrix_full = matrix_full.astype(np.int32)
        if if_Target_matrix_final(k,l,g,matrix_full):
            print("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!good matrix!")
            print(matrix_full)
            break
        if seed1%10000==0:
            print("seed1",seed1)
            
    
if __name__ == "__main__":
    k = 12
    l = 2
    g = 6
    #matrix = init_matrix(k,l,g)
    search_matrix(k,l,g)
    matrix = init_matrix_by_file("/home/msms/OOPPO/oppo_project/afile.dat")
    if_Target_matrix_final(k,l,g,matrix)
    