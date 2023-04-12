set -e

echo "实验4：测试bias" >> oppo_project/test.result
pkill -9 run_coordinator


echo "*******************************************" >> oppo_project/test.result
echo "8 4 3 Par_2_random 99" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./test_bias Par_2_random 8 4 3 16 99
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "8 4 3 Par_2_load 99" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./test_bias Par_2_load 8 4 3 16 99
pkill -9 run_coordinator
cd ../../..













echo "*******************************************" >> oppo_project/test.result
echo "8 4 3 Par_2_random 90" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./test_bias Par_2_random 8 4 3 16 90
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "8 4 3 Par_2_load 90" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./test_bias Par_2_load 8 4 3 16 90
pkill -9 run_coordinator
cd ../../..













echo "*******************************************" >> oppo_project/test.result
echo "8 4 3 Par_2_random 80" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./test_bias Par_2_random 8 4 3 16 80
pkill -9 run_coordinator
cd ../../..

echo "*******************************************" >> oppo_project/test.result
echo "8 4 3 Par_2_load 80" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./test_bias Par_2_load 8 4 3 16 80
pkill -9 run_coordinator
cd ../../..



