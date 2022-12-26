#ifndef TOOLBOX_H
#define TOOLBOX_H
#include <iostream>
#include <string>
#define MAX_KEY_LENGTH 200
#define MAX_VALUE_LENGTH 20000
namespace OppoProject {

extern bool random_generate_kv(std::string &key, std::string &value,
                               int key_length = 0, int value_length = 0);
} // namespace OppoProject
#endif