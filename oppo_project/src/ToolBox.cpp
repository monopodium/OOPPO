#include "toolbox.h"
#include <ctime>
#include<cstring>

bool OppoProject::random_generate_kv(std::string &key, std::string &value,
                                     int key_length, int value_length) {
  /*如果长度为0，则随机生成长度,key的长度不大于MAX_KEY_LENGTH，value的长度不大于MAX_VALUE_LENGTH*/
  /*现在生成的key,value内容是固定的，可以改成随机的(增加参数)*/
  /*如果需要生成的key太多，避免重复生成，可以改成写文件保存下来keyvalue，下次直接读文件的形式，
  但这个需要修改函数参数或者修改run_client的内容了*/

  srand((unsigned)time(0));
  if (key_length == 0) {
  } else {
    for (int i = 0; i < key_length; i++) {
      key = key + char('a' + rand() % 26);
    }
  }
  if (value_length == 0) {
  } else {
    for (int i = 0; i < value_length / 26; i++) {
      for (int j = 65; j <= 90; j++) {
        value = value + char(j);
      }
    }
    for (int i = 0; i < value_length - value.size(); i++) {
      value = value + char('A' + i);
    }
  }
  return true;
}

std::vector<unsigned char> OppoProject::int_to_bytes(int integer) {
  std::vector<unsigned char> bytes(sizeof(int));
  unsigned char *p = (unsigned char *)(&integer);
  for (int i = 0; i < bytes.size(); i++) {
    memcpy(&bytes[i], p + i, 1);
  }
  return bytes;
}
int OppoProject::bytes_to_int(std::vector<unsigned char> &bytes) {
  int integer;
  unsigned char *p = (unsigned char *)(&integer);
  for (int i = 0; i < bytes.size(); i++) {
    memcpy(p + i, &bytes[i], 1);
  }
  return integer;
}
// namespace OppoProject