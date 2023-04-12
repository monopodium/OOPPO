set -e

echo "实验3：不同编码参数，写入1GB数据，flat" >> oppo_project/test.result
pkill -9 run_coordinator
ssh mashuang@10.0.0.10 'cd wondershaper/;sudo ./wondershaper -c -a ib0;sudo ./wondershaper -a ib0 -d 3145728'
ssh mashuang@10.0.0.10 'cd wondershaper/;sudo ./wondershaper -c -a lo;sudo ./wondershaper -a lo -d 3145728'
ARRAY2=('10.0.0.9' '10.0.0.11' '10.0.0.12' '10.0.0.13' '10.0.0.14' '10.0.0.15' '10.0.0.16' '10.0.0.17' '10.0.0.18' '10.0.0.19')
NUM2=${#ARRAY2[@]}
NUM2=`expr $NUM2 - 1`
for i in $(seq 0 $NUM2)
do
    temp2=${ARRAY2[$i]}
    echo $temp2
    ssh mashuang@$temp2 'cd wondershaper/;sudo ./wondershaper -c -a ib0;sudo ./wondershaper -a ib0 -d 3145728'
    ssh mashuang@$temp2 'cd wondershaper/;sudo ./wondershaper -c -a lo;sudo ./wondershaper -a lo -d 3145728'
    sleep 2
done



echo "*******************************************" >> oppo_project/test.result
echo "8 4 3 Flat" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Random 8 4 3 1024
pkill -9 run_coordinator
cd ../../..









echo "*******************************************" >> oppo_project/test.result
echo "9 3 5 Flat" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Random 9 3 5 1024
pkill -9 run_coordinator
cd ../../..










echo "*******************************************" >> oppo_project/test.result
echo "10 5 3 Flat" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Random 10 5 3 1024
pkill -9 run_coordinator
cd ../../..








echo "*******************************************" >> oppo_project/test.result
echo "12 4 5 Flat" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Random 12 4 5 1024
pkill -9 run_coordinator
cd ../../..













echo "*******************************************" >> oppo_project/test.result
echo "14 7 5 Flat" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Random 14 7 5 1024
pkill -9 run_coordinator
cd ../../..












echo "*******************************************" >> oppo_project/test.result
echo "16 4 6 Flat" >> oppo_project/test.result
sh exp.sh 1;sh exp.sh 1
cd ./oppo_project/cmake/build/
./run_coordinator
sleep 2
./run_client Random 16 4 6 1024
pkill -9 run_coordinator
cd ../../..

