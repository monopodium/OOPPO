#ifndef META_DEFINITION
#define META_DEFINITION
#include "devcommon.h"
namespace OppoProject
{
  enum EncodeType
  {
    RS,
    OPPO_LRC,
    Azure_LRC_1
  };
  enum PlacementType
  {
    Random,
    Flat,
    Best_Placement
  };
  typedef std::unordered_map<int, std::unordered_map<int, std::vector<char>>> partial_helper;
  typedef struct AZitem
  {
    AZitem() : cur_node(0) {}
    unsigned int AZ_id;
    std::string proxy_ip;
    int proxy_port;
    std::vector<unsigned int> nodes;
    int cur_node;
  } AZitem;
  typedef struct Nodeitem
  {
    unsigned int Node_id;
    std::string Node_ip;
    int Node_port;
    int AZ_id;
    std::unordered_set<int> stripes;
  } Nodeitem;
  typedef struct ObjectItemBigSmall
  {
    bool big_object;
    int offset = -1;
    int shard_idx = -1;
    int object_size;
    std::vector<unsigned int> stripes;
  } ObjectItemBigSmall;

  typedef struct StripeItem
  {
    unsigned int Stripe_id;
    int shard_size;
    int k, real_l, g_m, b;
    std::vector<unsigned int> nodes;
    EncodeType encodetype;
    PlacementType placementtype;
  } StripeItem;

  typedef struct ECSchema
  {
    ECSchema() = default;

    ECSchema(bool partial_decoding, EncodeType encodetype, PlacementType placementtype, int k_datablock,
             int real_l_localgroup, int g_m_globalparityblock, int b_datapergoup, int small_file_upper, int blob_size_upper)
        : partial_decoding(partial_decoding), encodetype(encodetype), placementtype(placementtype),
          k_datablock(k_datablock), real_l_localgroup(real_l_localgroup),
          g_m_globalparityblock(g_m_globalparityblock), b_datapergoup(b_datapergoup),
          small_file_upper(small_file_upper), blob_size_upper(blob_size_upper) {}
    bool partial_decoding;
    EncodeType encodetype;
    PlacementType placementtype;
    int k_datablock;
    int real_l_localgroup;
    int g_m_globalparityblock;
    int b_datapergoup;
    int small_file_upper;
    int blob_size_upper;
  } ECSchema;

  typedef struct Range
  {
    int offset;
    int length;
    Range() = default;
    Range(int offset, int length) : offset(offset), length(length) {}
  } Range;

  typedef struct ShardidxRange
  {
    int shardidx;
    int offset_in_shard;
    int range_length;
    ShardidxRange() = default;
    ShardidxRange(int idx, int offset, int length) : shardidx(idx), offset_in_shard(offset), range_length(length) {}
  };
} // namespace OppoProject

#endif // META_DEFINITION