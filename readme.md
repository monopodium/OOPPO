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
├── include
│   ├── coordinator.h
│   ├── client.h
│   └── proxy.h
├── src
│   ├── ToolBox.cpp
│   ├── client.cpp
│   ├── coordinator.cpp
│   ├── memcached-1.6.17
|   ├── libmemcached-1.0.18
│   ├── proto
│   │   ├── coordinator.proto
│   │   └── proxy.proto
│   └── proxy.cpp
└── third_party
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
### 运行代码

```
cd oppo_project/cmake/build
./proxy
./client 127.0.0.1
./coordinator
```
### 提交代码步骤：
```c
git add .
git commit -m "commit message"
git push origin 分支名 # 注意，这里用自己的分支名，别把其它的分支给覆盖了
```

### 参考链接喔
https://grpc.io/docs/languages/cpp/quickstart/

切换gcc版本
https://zhuanlan.zhihu.com/p/261001751
