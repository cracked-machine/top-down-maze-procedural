#ifndef SRC_CMPS_PERSISTENT_CRYPTNPCSPAWNCOUNT_
#define SRC_CMPS_PERSISTENT_CRYPTNPCSPAWNCOUNT_

#include <Components/Npc/NpcShockwave.hpp>
#include <Components/Persistent/BasePersistent.hpp>
#include <cstdint>

namespace ProceduralMaze::Cmp::Persist
{

class CryptNpcSpawnCount : public BasePersistent<uint16_t>
{
public:
  CryptNpcSpawnCount( uint16_t value = 0, uint16_t min_value = 0, uint16_t max_value = 0 )
      : BasePersistent<uint16_t>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "CryptNpcSpawnCount"; }
  const std::string get_detail() const override { return ""; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // SRC_CMPS_PERSISTENT_CRYPTNPCSPAWNCOUNT_
