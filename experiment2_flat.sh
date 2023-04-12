set -e

echo "实验2：不同的跨集群带宽，写入1GB数据，flat" >> oppo_project/test.result
pkill -9 run_coordinator

ARRAY1=(6291456 4194304 3145728 2516582 2097152 1797558 1572864)
NUM1=${#ARRAY1[@]}
NUM1=`expr $NUM1 - 1`
ARRAY2=('10.0.0.9' '10.0.0.11' '10.0.0.12' '10.0.0.13' '10.0.0.14' '10.0.0.15' '10.0.0.16' '10.0.0.17' '10.0.0.18' '10.0.0.19')
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
        ssh mashuang@$temp2 'cd wondershaper/;sudo ./wondershaper -c -a lo;sudo ./wondershaper -a lo -d ' $temp1
        sleep 2
    done
    ssh mashuang@10.0.0.10 'cd wondershaper/;sudo ./wondershaper -c -a ib0;sudo ./wondershaper -a ib0 -d ' $temp1
    ssh mashuang@10.0.0.10 'cd wondershaper/;sudo ./wondershaper -c -a lo;sudo ./wondershaper -a lo -d ' $temp1
    echo "*******************************************" >> oppo_project/test.result
    echo "test_1MB_8_4_3_Flat" >> oppo_project/test.result
    echo $temp1 >> oppo_project/test.result
    sh exp.sh 1;sh exp.sh 1
    cd ./oppo_project/cmake/build/
    ./run_coordinator
    sleep 2
    ./run_client Random 8 4 3 1024
    pkill -9 run_coordinator
    cd ../../..

done