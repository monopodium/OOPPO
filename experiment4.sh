set -e

echo "实验4：测试bias" >> oppo_project/test.result
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


echo "*******************************************" >> oppo_project/test.result
echo "8 4 3" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./test_bias Random 8 4 3 8192 99
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "8 4 3" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./test_bias Best_Placement 8 4 3 8192 99
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "8 4 3" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./test_bias Best_Best_Placement 8 4 3 8192 99
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "8 4 3" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./test_bias Best_Best_Best_Placement 8 4 3 8192 99
pkill -9 run_coordinator
cd ../../..














echo "*******************************************" >> oppo_project/test.result
echo "8 4 3" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./test_bias Random 8 4 3 8192 90
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "8 4 3" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./test_bias Best_Placement 8 4 3 8192 90
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "8 4 3" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./test_bias Best_Best_Placement 8 4 3 8192 90
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "8 4 3" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./test_bias Best_Best_Best_Placement 8 4 3 8192 90
pkill -9 run_coordinator
cd ../../..
















echo "*******************************************" >> oppo_project/test.result
echo "8 4 3" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./test_bias Random 8 4 3 8192 80
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "8 4 3" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./test_bias Best_Placement 8 4 3 8192 80
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "8 4 3" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./test_bias Best_Best_Placement 8 4 3 8192 80
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "8 4 3" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./test_bias Best_Best_Best_Placement 8 4 3 8192 80
pkill -9 run_coordinator
cd ../../..



