#ifndef SRC_COMPONENTS_PERSISTENT_NPCWATCHMANSPAWNINFAMY_HPP__
#define SRC_COMPONENTS_PERSISTENT_NPCWATCHMANSPAWNINFAMY_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

class NpcWatchmanSpawnInfamy : public BasePersistent<uint8_t>
{
public:
  NpcWatchmanSpawnInfamy( uint8_t value = 0, uint8_t min_value = 0, uint8_t max_value = 0 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  [[nodiscard]] std::string class_name() const override { return "NpcWatchmanSpawnInfamy"; }
  [[nodiscard]] const std::string get_detail() const override { return detail; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_NPCWATCHMANSPAWNINFAMY_HPP__
