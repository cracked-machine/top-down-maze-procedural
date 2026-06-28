#ifndef SRC_COMPONENTS_PERSISTENT_NPCDEATHANIMFRAMERATE_HPP__
#define SRC_COMPONENTS_PERSISTENT_NPCDEATHANIMFRAMERATE_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

class NpcDeathAnimFramerate : public BasePersistent<float>
{
public:
  // time unit is seconds per frame, engine should convert to sf::Time as needed
  NpcDeathAnimFramerate( float seconds_per_frame = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( seconds_per_frame, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "NpcDeathAnimFramerate"; }
  const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_NPCDEATHANIMFRAMERATE_HPP__
