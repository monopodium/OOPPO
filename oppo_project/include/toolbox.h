#ifndef TOOLBOX_H
#define TOOLBOX_H
#include "meta_definition.h"
#define MAX_KEY_LENGTH 200
#define MAX_VALUE_LENGTH 20000
namespace OppoProject {

extern bool random_generate_kv(std::string key, std::string value,
                               int key_length, int value_length);
} // namespace OppoProject
#endif