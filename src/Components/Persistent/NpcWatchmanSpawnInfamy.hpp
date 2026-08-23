#ifndef SRC_COMPONENTS_PERSISTENT_NPCWATCHMANSPAWNINFAMY_HPP__
#define SRC_COMPONENTS_PERSISTENT_NPCWATCHMANSPAWNINFAMY_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

//! @brief Minimum player infamy required before a Watchman NPC will spawn.
class NpcWatchmanSpawnInfamy : public BasePersistent<uint8_t>
{
public:
  //! @brief Construct the Watchman spawn infamy threshold setting.
  //! @param value Initial infamy threshold.
  //! @param min_value Minimum allowed value (ImGui slider bound).
  //! @param max_value Maximum allowed value (ImGui slider bound).
  NpcWatchmanSpawnInfamy( uint8_t value = 0, uint8_t min_value = 0, uint8_t max_value = 0 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  //! @brief Returns the unique registry identifier for this persistent setting.
  [[nodiscard]] std::string class_name() const override { return "NpcWatchmanSpawnInfamy"; }
  //! @brief Returns the display description for this setting.
  [[nodiscard]] const std::string get_detail() const override { return detail; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_NPCWATCHMANSPAWNINFAMY_HPP__
