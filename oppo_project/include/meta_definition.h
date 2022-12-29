#ifndef META_DEFINITION
#define META_DEFINITION
#include "devcommon.h"

#define if_debug
// #define BOUNDARY_BIG_SMALL_OBJECT_BYTE 1048576 // 1M
// #define SHARD_SIZE_UPPER_BOUND_BYTE 2097152    // 2M

#define BOUNDARY_BIG_SMALL_OBJECT_BYTE 1024 // 1M
#define SHARD_SIZE_UPPER_BOUND_BYTE 2097152 // 2M
namespace OppoProject {

typedef struct DataNodeInfo {
  std::string nodeIP;
  std::string port;
  int nodeID;
} DataNodeInfo;
typedef struct AZitem {
  int AZ_id;
  std::vector<std::string> DataNodeInfo;
} AZitem;
enum EncodeType { RS, OPPO_LRC, Azure_LRC_1 };
enum PlacementType { Random, Flat, Best_Placement };
typedef struct ObjectItemBigSmall {
  bool big_object;
  int offset = -1;
  int object_size = -1;
  int block_size = 1024;
  std::vector<unsigned int> shard_id;
} ObjectItemBigSmall;

typedef struct ECSchema {
  ECSchema() = default;

  ECSchema(EncodeType encodetype, PlacementType placementtype, int k_datablock,
           int l_localgroup, int g_m_globalparityblock, int r_datapergoup)
      : encodetype(encodetype), placementtype{placementtype},
        k_datablock{k_datablock}, l_localgroup{l_localgroup},
        g_m_globalparityblock{g_m_globalparityblock}, r_datapergoup{
                                                          r_datapergoup} {}
  EncodeType encodetype = RS;
  PlacementType placementtype = Flat;
  int k_datablock = 3;
  int l_localgroup = 0;
  int g_m_globalparityblock = 2;
  int r_datapergoup = 0;
} ECSchema;
} // namespace OppoProject

#endif // META_DEFINITION