#ifndef SRC_CMPS_PERSISTENT_NPCSHOCKWAVERESOLUTION_
#define SRC_CMPS_PERSISTENT_NPCSHOCKWAVERESOLUTION_

#include <Components/NpcShockwave.hpp>
#include <Components/Persistent/BasePersistent.hpp>
#include <cstdint>

namespace ProceduralMaze::Cmp::Persist
{

class NpcShockwaveResolution : public BasePersistent<uint16_t>
{
public:
  NpcShockwaveResolution( uint16_t value = 48, uint16_t min_value = 32, uint16_t max_value = 64 )
      : BasePersistent<uint16_t>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "NpcShockwaveResolution"; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // SRC_CMPS_PERSISTENT_NPCSHOCKWAVERESOLUTION_
