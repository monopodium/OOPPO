set -e

echo "实验3：不同编码参数，写入4GB数据" >> oppo_project/test.result
pkill -9 run_coordinator
ssh mashuang@10.0.0.10 'cd wondershaper/;sudo ./wondershaper -c -a ib0;sudo ./wondershaper -a ib0 -d 1677721'
ARRAY2=('10.0.0.11' '10.0.0.12' '10.0.0.13' '10.0.0.14' '10.0.0.15' '10.0.0.16')
NUM2=${#ARRAY2[@]}
NUM2=`expr $NUM2 - 1`
for i in $(seq 0 $NUM2)
do
    temp2=${ARRAY2[$i]}
    echo $temp2
    ssh mashuang@$temp2 'cd wondershaper/;sudo ./wondershaper -c -a ib0;sudo ./wondershaper -a ib0 -d 1677721'
    sleep 2
done



# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..











# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..









# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..











echo "*******************************************" >> oppo_project/test.result
echo "24 4 11" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Random 24 4 11 8192
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "24 4 11" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Placement 24 4 11 8192
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "24 4 11" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Best_Placement 24 4 11 8192
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "24 4 11" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Best_Best_Placement 24 4 11 8192
pkill -9 run_coordinator
cd ../../..
















# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..










# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..











# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..









# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..











# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..
















# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..



















# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..











# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..









# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..











# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..
















# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..














# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..











# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..









# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..











# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..
















# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..
















# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..











# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..









# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..











# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..
















# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..
















# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..











# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..









# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..











# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..
















# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..
















# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..











# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..









# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..











# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..
















# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..























# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..











# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..









# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..











# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..
















# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..


























# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..











# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..









# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..











# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..
















# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..






















# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 8 4 3 8192
# pkill -9 run_coordinator
# cd ../../..











# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "9 3 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 9 3 5 8192
# pkill -9 run_coordinator
# cd ../../..









# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "12 4 5" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 12 4 5 8192
# pkill -9 run_coordinator
# cd ../../..











# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "24 4 11" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 24 4 11 8192
# pkill -9 run_coordinator
# cd ../../..
















# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Random 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "16 4 7" >> oppo_project/test.result
# sh exp.sh 1;sh exp.sh 1
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./run_client Best_Best_Best_Placement 16 4 7 8192
# pkill -9 run_coordinator
# cd ../../..