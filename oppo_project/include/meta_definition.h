#ifndef META_DEFINITION
#define META_DEFINITION
#include <devcommon.h>

#define if_debug
// #define BOUNDARY_BIG_SMALL_OBJECT_BYTE 1048576 // 1M
// #define SHARD_SIZE_UPPER_BOUND_BYTE 2097152    // 2M

#define BOUNDARY_BIG_SMALL_OBJECT_BYTE 1024 // 1M
#define SHARD_SIZE_UPPER_BOUND_BYTE 2097152 // 2M
namespace OppoProject {

struct DataNodeInfo {
  std::string nodeIP;
  std::string port;
  int nodeID;
};

enum EncodeType { RS, OPPO_LRC, Azure_LRC_1 };

typedef struct ObjectItemBigSmall {
  bool big_object;
  int offset = -1;
  int object_size = -1;
  std::vector<unsigned int> shard_id;
} ObjectItemBigSmall;

} // namespace OppoProject

#endif // META_DEFINITION