#ifndef SRC_CMPS_PERSISTENT_NPCSHOCKWAVEMAXRADIUS_
#define SRC_CMPS_PERSISTENT_NPCSHOCKWAVEMAXRADIUS_

#include <Components/Npc/NpcShockwave.hpp>
#include <Components/Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persist
{

class NpcShockwaveMaxRadius : public BasePersistent<float>
{
public:
  NpcShockwaveMaxRadius( float value = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "NpcShockwaveMaxRadius"; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // SRC_CMPS_PERSISTENT_NPCSHOCKWAVEMAXRADIUS_
