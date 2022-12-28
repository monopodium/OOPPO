## 友友们，从此就在这里写进度吧
### v0.0
实现了不大不小的文件RS没有放置策略的写

待实现：

...

等俺有力气了再整理一下

[已经修复]记录一个有趣的Bug:
当我在lambda表达式种用[object_and_placement]的方式传递变量，
object_and_placement->shardid()[0]输出的是这个140568642455760


再记一个无趣的bug:
这两个文件都包括了#include”dev_comm.h“
但是对于后面那个文件，#include”dev_comm.h“没有效果
