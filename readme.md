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

