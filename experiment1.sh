set -e

echo "实验1：不同的块大小，写入4GB数据" >> oppo_project/test.result
pkill -9 run_coordinator
ssh mashuang@10.0.0.10 'cd wondershaper/;sudo ./wondershaper -c -a ib0;sudo ./wondershaper -a ib0 -d 3355443'
ARRAY2=('10.0.0.11' '10.0.0.12' '10.0.0.13' '10.0.0.14' '10.0.0.15' '10.0.0.16')
NUM2=${#ARRAY2[@]}
NUM2=`expr $NUM2 - 1`
for i in $(seq 0 $NUM2)
do
    temp2=${ARRAY2[$i]}
    echo $temp2
    ssh mashuang@$temp2 'cd wondershaper/;sudo ./wondershaper -c -a ib0;sudo ./wondershaper -a ib0 -d 3355443'
    sleep 2
done



echo "*******************************************" >> oppo_project/test.result
echo "test_0.25MB_8_4_3_Random" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Random 8 4 3 256
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "test_0.25MB_8_4_3_Best" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Placement 8 4 3 256
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "test_0.25MB_8_4_3_Best_Best" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Best_Placement 8 4 3 256
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "test_0.25MB_8_4_3_Best_Best_Best" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Best_Best_Placement 8 4 3 256
pkill -9 run_coordinator
cd ../../..




echo "*******************************************" >> oppo_project/test.result
echo "test_0.5MB_8_4_3_Random" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Random 8 4 3 512
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "test_0.5MB_8_4_3_Best" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Placement 8 4 3 512
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "test_0.5MB_8_4_3_Best_Best" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Best_Placement 8 4 3 512
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "test_0.5MB_8_4_3_Best_Best_Best" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Best_Best_Placement 8 4 3 512
pkill -9 run_coordinator
cd ../../..
















echo "*******************************************" >> oppo_project/test.result
echo "test_0.75MB_8_4_3_Random" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Random 8 4 3 768
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "test_0.75MB_8_4_3_Best" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Placement 8 4 3 768
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "test_0.75MB_8_4_3_Best_Best" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Best_Placement 8 4 3 768
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "test_0.75MB_8_4_3_Best_Best_Best" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Best_Best_Placement 8 4 3 768
pkill -9 run_coordinator
cd ../../..










echo "*******************************************" >> oppo_project/test.result
echo "test_1MB_8_4_3_Random" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Random 8 4 3 1024
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "test_1MB_8_4_3_Best" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Placement 8 4 3 1024
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "test_1MB_8_4_3_Best_Best" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Best_Placement 8 4 3 1024
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "test_1MB_8_4_3_Best_Best_Best" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Best_Best_Placement 8 4 3 1024
pkill -9 run_coordinator
cd ../../..










echo "*******************************************" >> oppo_project/test.result
echo "test_4MB_8_4_3_Random" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Random 8 4 3 4096
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "test_4MB_8_4_3_Best" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Placement 8 4 3 4096
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "test_4MB_8_4_3_Best_Best" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Best_Placement 8 4 3 4096
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "test_4MB_8_4_3_Best_Best_Best" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Best_Best_Placement 8 4 3 4096
pkill -9 run_coordinator
cd ../../..

















echo "*******************************************" >> oppo_project/test.result
echo "test_8MB_8_4_3_Random" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Random 8 4 3 8192
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "test_8MB_8_4_3_Best" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Placement 8 4 3 8192
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "test_8MB_8_4_3_Best_Best" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Best_Placement 8 4 3 8192
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "test_8MB_8_4_3_Best_Best_Best" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Best_Best_Placement 8 4 3 8192
pkill -9 run_coordinator
cd ../../..














echo "*******************************************" >> oppo_project/test.result
echo "test_16MB_8_4_3_Random" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Random 8 4 3 16384
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "test_16MB_8_4_3_Best" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Placement 8 4 3 16384
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "test_16MB_8_4_3_Best_Best" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Best_Placement 8 4 3 16384
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "test_16MB_8_4_3_Best_Best_Best" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Best_Best_Placement 8 4 3 16384
pkill -9 run_coordinator
cd ../../..















echo "*******************************************" >> oppo_project/test.result
echo "test_32MB_8_4_3_Random" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Random 8 4 3 32768
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "test_32MB_8_4_3_Best" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Placement 8 4 3 32768
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "test_32MB_8_4_3_Best_Best" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Best_Placement 8 4 3 32768
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "test_32MB_8_4_3_Best_Best" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Best_Best_Best_Placement 8 4 3 32768
pkill -9 run_coordinator
cd ../../..