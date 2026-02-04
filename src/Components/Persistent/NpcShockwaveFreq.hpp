#ifndef SRC_CMPS_PERSISTENT_NPCSHOCKWAVEFREQ_
#define SRC_CMPS_PERSISTENT_NPCSHOCKWAVEFREQ_

#include <Components/Npc/NpcShockwave.hpp>
#include <Components/Persistent/BasePersistent.hpp>
#include <cstdint>

namespace ProceduralMaze::Cmp::Persist
{

class NpcShockwaveFreq : public BasePersistent<uint16_t>
{
public:
  NpcShockwaveFreq( uint16_t value = 0, uint16_t min_value = 0, uint16_t max_value = 0 )
      : BasePersistent<uint16_t>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "NpcShockwaveSpeed"; }
  const std::string get_detail() const override { return ""; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // SRC_CMPS_PERSISTENT_NPCSHOCKWAVEFREQ_
