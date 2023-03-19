#!/bin/bash
set -e
# DIR_NAME=move_and_run
# rm -r $DIR_NAME
# mkdir $DIR_NAME
# cp ./AZ_run_memcached.sh ./$DIR_NAME
# cp ./AZ_run_proxy_datanode.sh ./$DIR_NAME
# cp -r ./memcached ./$DIR_NAME

ARRAY=('10.0.0.11' '10.0.0.12' '10.0.0.13' '10.0.0.14' '10.0.0.15' '10.0.0.16')
NUM=${#ARRAY[@]}
echo "cluster_number:"$NUM
NUM=`expr $NUM - 1`
#SRC_PATH1=/home/mashuang/ooooppo/OOPPO
SRC_PATH2=/home/mashuang/ooooppo/OOPPO/memcached
SRC_PATH3=/home/mashuang/ooooppo/OOPPO/AZ_run_memcached.sh
SRC_PATH4=/home/mashuang/ooooppo/OOPPO/AZ_run_proxy_datanode.sh
SRC_PATH5=/home/mashuang/ooooppo/OOPPO/oppo_project

# DIR_NAME=run_memcached
DIS_DIR1=/home/mashuang/ooooppo/OOPPO
DIS_DIR2=/home/mashuang/ooooppo/OOPPO/oppo_project

for i in $(seq 0 $NUM)
do
temp=${ARRAY[$i]}
    echo $temp
    if [ $1 == 0 ]
    then
        ssh mashuang@$temp 'pkill  -9 oppo_memcached;pkill  -9 run_datanode;pkill  -9 run_proxy'
        echo 'pkill  all'
        ssh mashuang@$temp 'ps -aux |grep oppo_memcached | wc -l'
    else
        if [ $1 == 1 ]
        then
            sudo yum install libevent-devel
            #sudo yum install pidof
            #ssh mashuang@$temp 'rm -r' ${DIS_DIR1}
            ssh mashuang@$temp 'mkdir -p' ${DIS_DIR1}
            ssh mashuang@$temp 'mkdir -p' ${DIS_DIR2}
            rsync -rtvpl ${SRC_PATH2} mashuang@$temp:${DIS_DIR1}
            rsync -rtvpl ${SRC_PATH3} mashuang@$temp:${SRC_PATH3}
            rsync -rtvpl ${SRC_PATH4} mashuang@$temp:${SRC_PATH4}
            rsync -rtvpl ${SRC_PATH5} mashuang@$temp:${DIS_DIR1}
        fi
        ssh mashuang@$temp 'hostname'
        ssh mashuang@$temp 'cd /home/mashuang/ooooppo/OOPPO;bash AZ_run_memcached.sh;bash AZ_run_proxy_datanode.sh'
        echo 'memcached process number:'
        ssh mashuang@$temp 'ps -aux |grep oppo_memcached | wc -l;ps -aux |grep run_datanode | wc -l;ps -aux |grep run_proxy | wc -l'
    fi
done