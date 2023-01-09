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
  std::string proxy;
  std::vector<int> DataNodeInfo;
} AZitem;
typedef struct Nodeitem {
  int Node_id;
  std::string ip_port;
  int AZ_id;  
} Nodeitem;
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

  ECSchema(bool partial_decoding, EncodeType encodetype, PlacementType placementtype, int k_datablock,
           int l_localgroup, int g_m_globalparityblock, int r_datapergoup, int small_file_upper, int blob_size_upper)
      : partial_decoding(partial_decoding), encodetype(encodetype), placementtype(placementtype),
        k_datablock(k_datablock), l_localgroup(l_localgroup),
        g_m_globalparityblock(g_m_globalparityblock), r_datapergoup(r_datapergoup), 
        small_file_upper(small_file_upper), blob_size_upper(blob_size_upper) {}
  bool partial_decoding;
  EncodeType encodetype;
  PlacementType placementtype;
  int k_datablock;
  int l_localgroup;
  int g_m_globalparityblock;
  int r_datapergoup;
  int small_file_upper;
  int blob_size_upper;
} ECSchema;
} // namespace OppoProject

#endif // META_DEFINITION