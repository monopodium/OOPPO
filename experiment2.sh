set -e

echo "实验2：不同的跨集群带宽，写入4GB数据" >> oppo_project/test.result
pkill -9 run_coordinator

ARRAY1=(13421773 6710886 3355443 1677721 838860 419430)
# ARRAY1=(33554432 13421773 6710886 3355443)
NUM1=${#ARRAY1[@]}
NUM1=`expr $NUM1 - 1`
ARRAY2=('10.0.0.11' '10.0.0.12' '10.0.0.13' '10.0.0.14' '10.0.0.15' '10.0.0.16')
NUM2=${#ARRAY2[@]}
NUM2=`expr $NUM2 - 1`



for i in $(seq 0 $NUM1)
do
    temp1=${ARRAY1[$i]}
    echo $temp1
    for i in $(seq 0 $NUM2)
    do
        temp2=${ARRAY2[$i]}
        echo $temp2
        ssh mashuang@$temp2 'cd wondershaper/;sudo ./wondershaper -c -a ib0;sudo ./wondershaper -a ib0 -d ' $temp1
        sleep 2
    done
    ssh mashuang@10.0.0.10 'cd wondershaper/;sudo ./wondershaper -c -a ib0;sudo ./wondershaper -a ib0 -d ' $temp1
    echo "*******************************************" >> oppo_project/test.result
    echo "test_8MB_8_4_3_Random" >> oppo_project/test.result
    echo $temp1 >> oppo_project/test.result
    sh exp.sh 1;sh exp.sh 1
    cd ./oppo_project/cmake/build/
    ./run_coordinator
    sleep 2
    ./run_client Random 8 4 3 8192
    pkill -9 run_coordinator
    cd ../../..

    echo "*******************************************" >> oppo_project/test.result
    echo "test_8MB_8_4_3_Best" >> oppo_project/test.result
    echo $temp1 >> oppo_project/test.result
    sh exp.sh 1;sh exp.sh 1
    cd ./oppo_project/cmake/build/
    ./run_coordinator
    sleep 2
    ./run_client Best_Placement 8 4 3 8192
    pkill -9 run_coordinator
    cd ../../..

    echo "*******************************************" >> oppo_project/test.result
    echo "test_8MB_8_4_3_Best_Best" >> oppo_project/test.result
    echo $temp1 >> oppo_project/test.result
    sh exp.sh 1;sh exp.sh 1
    cd ./oppo_project/cmake/build/
    ./run_coordinator
    sleep 2
    ./run_client Best_Best_Placement 8 4 3 8192
    pkill -9 run_coordinator
    cd ../../..

    echo "*******************************************" >> oppo_project/test.result
    echo "test_8MB_8_4_3_Best_Best_Best" >> oppo_project/test.result
    echo $temp1 >> oppo_project/test.result
    sh exp.sh 1;sh exp.sh 1
    cd ./oppo_project/cmake/build/
    ./run_coordinator
    sleep 2
    ./run_client Best_Best_Best_Placement 8 4 3 8192
    pkill -9 run_coordinator
    cd ../../..
done