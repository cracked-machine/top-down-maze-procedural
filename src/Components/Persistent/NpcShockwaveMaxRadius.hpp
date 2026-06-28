#ifndef SRC_COMPONENTS_PERSISTENT_NPCSHOCKWAVEMAXRADIUS_HPP__
#define SRC_COMPONENTS_PERSISTENT_NPCSHOCKWAVEMAXRADIUS_HPP__

#include <Components/Npc/NpcShockwave.hpp>
#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

class NpcShockwaveMaxRadius : public BasePersistent<float>
{
public:
  NpcShockwaveMaxRadius( float value = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "NpcShockwaveMaxRadius"; }
  const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_NPCSHOCKWAVEMAXRADIUS_HPP__
