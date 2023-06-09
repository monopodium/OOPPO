## OPPO项目代码（纯享版
### 仓库规范
* 建议友友们用clang-format规范代码
https://zhuanlan.zhihu.com/p/356143396
* 新的代码可以各自新建分支，先提交到自己的分支，
再提交Pull request, 另外一个友友审核（也许没有这个环节）之后再merge到main分支。
* 不需要的文件，例如编译结果，可以通过gitignore忽略。
* 踩过的坑和解决方案可以写在仓库里
* 也许可以设置一些检查，在提交前自动检测格式，但俺不会，有无友友教教


### 代码框架和库选择
* 初步打算用grpc(传输元数据信息）+socket（传输数据），这是之前的一个可以参考的项目：https://github.com/monopodium/code

### 小文件EC的三篇实现参考
* 感觉小文件EC需要提供一些从一个block的某个offset开始读的函数，但是memcached的get(key)只能一下子读。
* Cocytus：修改了memcached的源码，但好像没有类似的功能，没太看懂（
* MemEC: 并没有基于memcached实现。
* fragEC: 小文件只是在逻辑上聚合为一个block,实际上还是分散存储在各个datanode。
* 现在想到有两种思路可以选择：1.改memcached源码。2.把这个功能移交给proxy，不改memcached源码，proxy读取一个block,然后读对应offset的内容。

### Log Manager
不懂捏
### 原型
oppo_project是项目的根目录，文件结构组织如下：
```
.
├── CMakeLists.txt
├── compile.sh
├── compile_without_thirdp.sh
├── config
│   └── AZInformation.xml
├── include
│   ├── azure_lrc.h
│   ├── client.h
│   ├── coordinator.h
│   ├── datanode.h
│   ├── devcommon.h
│   ├── meta_definition.h
│   ├── proxy.h
│   ├── tinyxml2.h
│   └── toolbox.h
├── run_client.cpp
├── run_coordinator.cpp
├── run_datanode.cpp
├── run_proxy.cpp
├── test_tools.cpp
├── src
│   ├── ToolBox.cpp
│   ├── azure_lrc.cpp
│   ├── client.cpp
│   ├── coordinator.cpp
│   ├── datanode.cpp
│   ├── proto
│   │   ├── coordinator.grpc.pb.cc
│   │   ├── coordinator.grpc.pb.h
│   │   ├── coordinator.pb.cc
│   │   ├── coordinator.pb.h
│   │   ├── coordinator.proto
│   │   ├── proxy.grpc.pb.cc
│   │   ├── proxy.grpc.pb.h
│   │   ├── proxy.pb.cc
│   │   ├── proxy.pb.h
│   │   └── proxy.proto
│   ├── proxy.cpp
│   └── tinyxml2.cpp
└── third_party
    ├── ycsb-0.12.0
    ├── jerasure
    ├── gf-complete
    ├── libmemcached
    ├── grpc
    └── asio
```
* 其中，考虑到可能修改libmemcached和memcached的源码，所以将这两部分的源码放置在了/src/文件夹下，和大部分项目一样src保存的源文件，include保存的是头文件。
* 不要让所有的代码都只写在proxy、coordinator、client三个文件里捏，需要合理的划分。
* oppo_project/third_party文件夹包含了第三方库：libmemcached、asio、grpc
compile.sh；但他们生成他们的时机不同，grpc和asio是真正当做第三方库安装的（不需要修改源代码）。第三方库也会越来越多。
* compile.sh是为了编译整个项目，其中，分为三部分，第一部分是为了编译Libmemcached，安装到oppo_project/third_party中，第二部分是为了编译memcached，安装在OOPO/memcached下;
第三部分是用cmakelist组织的。


### 环境配置
又到了万恶的环境配置阶段啦！

是这样的，项目目前需要安装的玩意儿如下：
* libmemcached: v1.0.18
* memcached: 1.6.17
* grpc v1.50
* asio 1.24.0
* ycsb 0.12.0 (测试用)

当前稳定的GCC与CMake版本
* gcc 9.4.0
* cmake 3.22.0

注意为了卸载方便，所有的东西都建议安装在局部的目录，为了避免由于众所周知的原因grpc源码难以下载，我们提供了打包好的源代码，可以运行脚本一键安装。
```
#grpc依赖的库：
sudo apt install -y build-essential autoconf libtool pkg-config
cd OOPPO
sh install_third_party_offline.sh
```
### 编译项目
```
cd oppo_project
sh compile.sh
```
### 运行代码(最新的参数定义在测试，这里的不灵去下面找)

```
sh run_memcached.sh
sh run_proxy_datanode.sh
cd oppo_project/cmake/build
./run_coordinator
./run_client false RS Random 3 -1 2 1024 4096 random 2048
./run_client false OPPO_LRC Random 12 3 6 1024 4096 ycsb 2048
./run_client false Azure_LRC_1 Random 12 2 6 1024 4096 ycsb 2048

```
### 提交代码步骤：
```c
git add .
git commit -m "commit message"
git push origin 分支名 # 注意，这里用自己的分支名，别把其它的分支给覆盖了
```

### 运行和关闭memcached
```c
./memcached/bin/memcached -m 128 -p 8100 --max-item-size=5242880 -vv -d
ps -ef|grep memcached
pkill -9 memcached
```
### 一些小坑坑
* gprc有可能会掩盖掉可能出现的报错，比如调用了一个rpc函数，函数里面有一句报错，但不会提示的，这个时候要善用
```c
  try {}
  catch(std::exception &e){
    std::cout << "exception" << std::endl;
    std::cout << e.what() << std::endl;
  }
```
* 没有用什么高级的异步通信手段，因此，开了一个socket等数据的话，会一直等喔，因此这里用了线程

### 测试
参数含义
```c
./run_client partial_decoding encode_type placement_type k l g small_file_upper blob_size_upper trace_type file_size
```
实际例子
```c
sh run_memcached.sh
sh run_proxy_datanode.sh
./run_coordinator
./run_client false RS Random 12 -1 6 1024 4096 random 2048
./run_client false OPPO_LRC Random 12 3 6 1024 4096 ycsb 2048
./run_client false Azure_LRC_1 Random 12 2 6 1024 4096 ycsb 2048
```

#### 更新测试
使用nohup，将守护进程proxy\datanode的输出输出到/log文件夹下的log文件
在OPPO文件夹下`sh autoall.sh`启动proxy和datanode
之后启动run_coordinatror  可以在build下运行 `./run_coordinator > ../../../log/coordinator.log 2>&1 &`
在OPPO文件夹下 `sh run_rmw.sh` 启动测试，参数最后一个为更新方式，可选RCW,RMW,AZ_Coordinated。更新的长度位置是写死的，之后再改一下client发送时的






为了测试repair操作，将AZ和proxy增加到了10，数据节点增加到了100
因为proxy数量较多，所以写成了守护进程的形式，使用run_proxy_datanode.sh脚本启动
为了避免memcached的输出信息干扰proxy和datanode的输出信息，将proxy和datanode的启动都放到了run_proxy_datanode.sh中
测试前建议先看一看
* 现在AZInformation.xml和run_memcached.sh和run_memcached.sh都可以由/small_tools的脚本生成了，生成逻辑与之前手写的方式一样，使用方法：
```
python small_tools/generator_sh.py
```
* generator_sh.py文件中的逻辑：同一个AZ中的Proxy和datanode以及memcached的ip相同端口不同，可以通过proxy_ip_list修改proxy的Ip
* AZInformation.xml配置文件
* run_memcached.sh: run_memcached.sh会开启很多memcached进程，可以通过以下命令快速杀死
* run_proxy.sh: run_proxy.sh会开启很多proxy进程和datanode进程，注意，datanode进程和memcached进程是一一对应的


  ```c
  kill -9 $(pidof run_datanode)
  kill -9 $(pidof memcached)
  kill -9 $(pidof run_proxy)
  ```
* 为了避免不必要的编译过程（只编译/src完全自己手写的代码）：
  ```c
  sh compile_without_thirdp.sh
  ```

* 后续应该改成下面这种形式，以指定coordinator或proxy的地址,but还没改（2023.2.1）：
  ```
  ./run_coordinator ip:port
  ./run_proxy ip:port
  ```

### ycsb的安装（只生成trace可以不安装ycsb官方二进制包）
安装过程比较简单，官方已经提供了编译好的二进制包：
```c
curl -O --location https://github.com/brianfrankcooper/YCSB/releases/download/0.12.0/ycsb-0.12.0.tar.gz
tar xfvz ycsb-0.12.0.tar.gz
cd ycsb-0.12.0
```

执行方式如下：（解压后文件置于"OOPPO/oppo_project/third_party"中）
```c
./bin/ycsb
```

### ycsb-trace生成
参考链接：https://haslab.org/2021/07/14/YCSB_trace.html
以下脚本用于生成ycsb-trace:
```c
git clone git@github.com:has-lab/YCSB-tracegen.git
cd YCSB-tracegen
mvn -pl site.ycsb:rocksdb-binding -am clean package
./ycsb.sh
```
生成的trace文件包含两个：
* YCSB-tracegen/warm.txt（load阶段预先插入的KV）
* YCSB-tracegen/test.txt（run阶段的访问模式）。

使用的负载为workloadc,可以在YCSB-tracegen/workloads/workloadc文件中修改其参数设置,其他workload可以通过修改./ycsb.sh指定。

### 参考链接喔
https://grpc.io/docs/languages/cpp/quickstart/

切换gcc版本
https://zhuanlan.zhihu.com/p/261001751

https://bugs.launchpad.net/libmemcached

