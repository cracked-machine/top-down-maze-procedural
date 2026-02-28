#ifndef SRC_CMPS_PERSISTENT_NPCSKELEGRAVEYARDSPAWNCOUNT_HPP_
#define SRC_CMPS_PERSISTENT_NPCSKELEGRAVEYARDSPAWNCOUNT_HPP_

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persist
{

class NpcSkeleGraveyardSpawnCount : public BasePersistent<uint8_t>
{
public:
  NpcSkeleGraveyardSpawnCount( uint8_t value = 0, uint8_t min_value = 0, uint8_t max_value = 0 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "NpcSkeleGraveyardSpawnCount"; }
  const std::string get_detail() const override { return detail; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // SRC_CMPS_PERSISTENT_NPCSKELEGRAVEYARDSPAWNCOUNT_HPP_
