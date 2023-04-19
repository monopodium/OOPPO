set -e

echo "实验4more：测试bias" >> oppo_project/test.result
pkill -9 run_coordinator




echo "*******************************************" >> oppo_project/test.result
echo "8 4 3 Par_2_random 90" >> oppo_project/test.result
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./test_bias Par_2_random 8 4 3 16 90 50
pkill -9 run_coordinator
cd ../../..



# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3 Par_2_load 90 0" >> oppo_project/test.result
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./test_bias Par_2_load 8 4 3 16 90 0
# pkill -9 run_coordinator
# cd ../../..



# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3 Par_2_load 90 25" >> oppo_project/test.result
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./test_bias Par_2_load 8 4 3 16 90 25
# pkill -9 run_coordinator
# cd ../../..

# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3 Par_2_load 90 50" >> oppo_project/test.result
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./test_bias Par_2_load 8 4 3 16 90 50
# pkill -9 run_coordinator
# cd ../../..













# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3 Par_2_load 90 75" >> oppo_project/test.result
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./test_bias Par_2_load 8 4 3 16 90 75
# pkill -9 run_coordinator
# cd ../../..



# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3 Par_2_load 90 100" >> oppo_project/test.result
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./test_bias Par_2_load 8 4 3 16 90 100
# pkill -9 run_coordinator
# cd ../../..











# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3 Par_2_load 90 100" >> oppo_project/test.result
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./test_bias Par_2_load 8 4 3 16 90 100
# pkill -9 run_coordinator
# cd ../../..


# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3 Par_2_load 90 90" >> oppo_project/test.result
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./test_bias Par_2_load 8 4 3 16 90 90
# pkill -9 run_coordinator
# cd ../../..


# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3 Par_2_load 90 89" >> oppo_project/test.result
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./test_bias Par_2_load 8 4 3 16 90 89
# pkill -9 run_coordinator
# cd ../../..


# echo "*******************************************" >> oppo_project/test.result
# echo "8 4 3 Par_2_load 90 88" >> oppo_project/test.result
# cd ./oppo_project/cmake/build/
# ./run_coordinator
# sleep 2
# ./test_bias Par_2_load 8 4 3 16 90 88
# pkill -9 run_coordinator
# cd ../../..