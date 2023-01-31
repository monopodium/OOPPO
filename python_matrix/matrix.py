# Linear Algebra Learning Sequence
# Vandermonde Matrix
 
import numpy as np
import math
import itertools
import galois
from itertools import product
from tqdm import tqdm
import random

two_w = 2**12
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

    GF = galois.GF(two_w)
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
    
    GF = galois.GF(two_w)
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
    GF = galois.GF(two_w)
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
    GF = galois.GF(two_w)
    seed1 = 0
    while(1):
        # x = np.array(range(2,k+2))
        # x = x.view(GF)
        seed1 = seed1 + 1
        x = GF.Random(k, low=0, high=255, seed=seed1)
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
            
def search_matrix_iteration(k,l,g):
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
    GF = galois.GF(two_w)
    seed1 = 0

    while(1):
        # x = np.array(range(2,k+2))
        # x = x.view(GF)
        seed1 = seed1 + 1
        x1 = GF.Random(int(k/2), low=0, high=int(two_w/2)-1, seed=seed1)
        x2 = GF.Random(k - int(k/2), low=int(two_w/2), high=int(two_w), seed=seed1)

        x = np.hstack((x1,x2))
        #x = np.array([253,129,237,112,144,34,196,136,222,15,139,131])
        
        A = [2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,\
            179,181,191,193,197,199,211,223,227,229,233,239,241,251,257,263,269,271,277,281,283,293,307,311,313,317,331,337,347,349,353,359,367,373,379,383,389,397,401,\
                409,419,421,431,433,439,443,449,457,461,463,467,479,487,491,499,503,509,521,523,541,547,557,563,569,571,577,587,593,599,601,607,613,617,619,631,641,643,647,\
                    653,659,661,673,677,683,691,701,709,719,727,733,739,743,751,757,761,769,773,787,797,809,811,821,823,827,829,839,853,857,859,863,877,881,883,887,907,911,919,\
                        929,937,941,947,953,967,971,977,983,991,997,1009,1013,1019,1021,1031,1033,1039,1049,1051,1061,1063,1069,1087,1091,1093,1097,1103,1109,1117,1123,1129,1151,\
                            1153,1163,1171,1181,1187,1193,1201,1213,1217,1223,1229,1231,1237,1249,1259,1277,1279,1283,1289,1291,1297,1301,1303,1307,1319,1321,1327,1361,1367,1373,1381,\
                                1399,1409,1423,1427,1429,1433,1439,1447,1451,1453,1459,1471,1481,1483,1487,1489,1493,1499,1511,1523,1531,1543,1549,1553,1559,1567,1571,1579,1583,1597,1601,\
                                    1607,1609,1613,1619,1621,1627,1637,1657,1663,1667,1669,1693,1697,1699,1709,1721,1723,1733,1741,1747,1753,1759,1777,1783,1787,1789,1801,1811,1823,1831,1847,\
                                        1861,1867,1871,1873,1877,1879,1889,1901,1907,1913,1931,1933,1949,1951,1973,1979,1987,1993,1997,1999,2003,2011,2017,2027,2029,2039,2053,2063,2069,2081,2083,\
                                            2087,2089,2099,2111,2113,2129,2131,2137,2141,2143,2153,2161,2179,2203,2207,2213,2221,2237,2239,2243,2251,2267,2269,2273,2281,2287,2293,2297,2309,2311,2333,\
                                                2339,2341,2347,2351,2357,2371,2377,2381,2383,2389,2393,2399,2411,2417,2423,2437,2441,2447,2459,2467,2473,2477,2503,2521,2531,2539,2543,2549,2551,2557,2579,\
                                                    2591,2593,2609,2617,2621,2633,2647,2657,2659,2663,2671,2677,2683,2687,2689,2693,2699,2707,2711,2713,2719,2729,2731,2741,2749,2753,2767,2777,2789,2791,2797,2801,\
                                                        2803,2819,2833,2837,2843,2851,2857,2861,2879,2887,2897,2903,2909,2917,2927,2939,2953,2957,2963,2969,2971,2999,3001,3011,3019,3023,3037,3041,3049,3061,3067,3079,3083,\
                                                            3089,3109,3119,3121,3137,3163,3167,3169,3181,3187,3191,3203,3209,3217,3221,3229,3251,3253,3257,3259,3271,3299,3301,3307,3313,3319,3323,3329,3331,3343,3347,3359,3361,\
                                                                3371,3373,3389,3391,3407,3413,3433,3449,3457,3461,3463,3467,3469,3491,3499,3511,3517,3527,3529,3533,3539,3541,3547,3557,3559,3571,3581,3583,3593,3607,3613,3617,3623,\
                                                                    3631,3637,3643,3659,3671,3673,3677,3691,3697,3701,3709,3719,3727,3733,3739,3761,3767,3769,3779,3793,3797,3803,3821,3823,3833,3847,3851,3853,3863,3877,3881,3889,3907,3911,3917,3919,\
                                                                        3923,3929,3931,3943,3947,3967,3989,4001,4003,4007,4013,4019,4021,4027,4049,4051,4057,4073,4079,4091,4093]
        x = np.array(random.sample(A,k))
        x = np.array([2,3,5,7,11,13])
        x = x.view(GF)
        x = np.hstack((x,GF.Random(int(k/2), low=0, high=15, seed=seed1)))
        print(x)
        x3 = np.hstack((x1,x2))
        x3 = GF.Random(k, low=0, high=int(two_w), seed=seed1)
        
        x3 = x3.view(GF)
        x = x/x3
        flag = 0
        # for item in x:
        #     for item1 in x:
        #         if galois.gcd(int(item1),int(item))==1 or item1==item:
        #             pass
        #         else:
        #             flag = 1
        # if flag == 1:
        #     continue
        
        matrix_g = []
        x_n = np.array([1]*k)
        x_n = x_n.view(GF)
        for i in range(g):
            x_n = x_n*x
            matrix_g.append(x_n)
        matrix_g = np.array(matrix_g).view(GF)
        matrix_full = np.vstack((np.vstack((matrix_k,matrix_l)),matrix_g))
        matrix_full = matrix_full.astype(np.int32)
        #print(matrix_full)
        if if_Target_matrix_final(k,l,g,matrix_full):
            print("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!good matrix!")
            print(matrix_full)
            break
        if seed1%10000==0:
            print("seed1",seed1)    
    
if __name__ == "__main__":
    k = 12
    l = 2
    g = 3
    #matrix = init_matrix(k,l,g)
    search_matrix(k,l,g)
    search_matrix_iteration(k,l,g)
    #matrix = init_matrix_by_file("/home/msms/OOPPO/oppo_project/afile.dat")
    #if_Target_matrix_final(k,l,g,matrix)
    