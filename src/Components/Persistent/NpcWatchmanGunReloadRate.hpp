#ifndef SRC_COMPONENTS_PERSISTENT_NPCWATCHMANGUNRELOADRATE_HPP__
#define SRC_COMPONENTS_PERSISTENT_NPCWATCHMANGUNRELOADRATE_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

//! @brief Rate at which the Watchman NPC's gun reloads (fire-to-cock delay).
class NpcWatchmanGunReloadRate : public BasePersistent<float>
{
public:
  //! @brief Construct the Watchman gun reload rate setting.
  //! @param value Initial reload rate.
  //! @param min_value Minimum allowed value (ImGui slider bound).
  //! @param max_value Maximum allowed value (ImGui slider bound).
  NpcWatchmanGunReloadRate( float value = 0, float min_value = 0, float max_value = 0 )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  //! @brief Returns the unique registry identifier for this persistent setting.
  [[nodiscard]] std::string class_name() const override { return "NpcWatchmanGunReloadRate"; }
  //! @brief Returns the display description for this setting.
  [[nodiscard]] const std::string get_detail() const override { return detail; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_NPCWATCHMANGUNRELOADRATE_HPP__
