oppo_project是项目本身，里面包含了proxy、coordinator、client等的源代码
third_party文件夹包含了libmemcached、asio、grpc的源代码
third_party_install是上述的库编译安装的路径

clean.sh文件将三个库文件的源代码还原到初始状态并删除已经安装的库
install_third_party.sh文件可以将三个库安装到third_party_install
如果更新了libmemcached的代码，在提交前请务必执行update_libmemcached.sh

grpc前置条件：
sudo apt install -y build-essential autoconf libtool pkg-config

环境配置步骤：
git clone git@github.com:monopodium/OOPPO.git -b zh_develop
cd OOPPO
./clean.sh
./install_third_party.sh
cd oppo_project
mkdir build
cd build
cmake ..
make -j

提交代码步骤：
回到OOPPO目录下
./update_libmemcached.sh
./gitadd.sh
git commit -m "commit message"
git push origin 分支名 # 注意，这里用自己的分支名，别把其它的分支给覆盖了

