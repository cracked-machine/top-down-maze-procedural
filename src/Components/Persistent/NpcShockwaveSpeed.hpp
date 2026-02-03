#ifndef SRC_CMPS_PERSISTENT_NPCSHOCKWAVESPEED_
#define SRC_CMPS_PERSISTENT_NPCSHOCKWAVESPEED_

#include <Components/Npc/NpcShockwave.hpp>
#include <Components/Persistent/BasePersistent.hpp>
#include <cstdint>

namespace ProceduralMaze::Cmp::Persist
{

class NpcShockwaveSpeed : public BasePersistent<uint16_t>
{
public:
  NpcShockwaveSpeed( uint16_t value = 0, uint16_t min_value = 0, uint16_t max_value = 0 )
      : BasePersistent<uint16_t>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "NpcShockwaveSpeed"; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // SRC_CMPS_PERSISTENT_NPCSHOCKWAVESPEED_
