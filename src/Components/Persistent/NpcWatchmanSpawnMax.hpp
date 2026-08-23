#ifndef SRC_COMPONENTS_PERSISTENT_NPCWATCHMANSPAWNMAX_HPP__
#define SRC_COMPONENTS_PERSISTENT_NPCWATCHMANSPAWNMAX_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

//! @brief Maximum number of Watchman NPCs allowed to exist simultaneously.
class NpcWatchmanSpawnMax : public BasePersistent<uint8_t>
{
public:
  //! @brief Construct the Watchman max spawn count setting.
  //! @param value Initial maximum count.
  //! @param min_value Minimum allowed value (ImGui slider bound).
  //! @param max_value Maximum allowed value (ImGui slider bound).
  NpcWatchmanSpawnMax( uint8_t value = 0, uint8_t min_value = 0, uint8_t max_value = 0 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  //! @brief Returns the unique registry identifier for this persistent setting.
  [[nodiscard]] std::string class_name() const override { return "NpcWatchmanSpawnMax"; }
  //! @brief Returns the display description for this setting.
  [[nodiscard]] const std::string get_detail() const override { return detail; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_NPCWATCHMANSPAWNMAX_HPP__
