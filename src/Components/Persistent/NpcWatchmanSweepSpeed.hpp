#ifndef SRC_COMPONENTS_PERSISTENT_NPCWATCHMANSWEEPSPEED_HPP__
#define SRC_COMPONENTS_PERSISTENT_NPCWATCHMANSWEEPSPEED_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

//! @brief Speed of the Watchman NPC's searchlight/cone sweep motion.
class NpcWatchmanSweepSpeed : public BasePersistent<float>
{
public:
  //! @brief Construct the Watchman sweep speed setting.
  //! @param value Initial sweep speed.
  //! @param min_value Minimum allowed value (ImGui slider bound).
  //! @param max_value Maximum allowed value (ImGui slider bound).
  NpcWatchmanSweepSpeed( float value = 0, float min_value = 0, float max_value = 0 )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  //! @brief Returns the unique registry identifier for this persistent setting.
  [[nodiscard]] std::string class_name() const override { return "NpcWatchmanSweepSpeed"; }
  //! @brief Returns the display description for this setting.
  [[nodiscard]] const std::string get_detail() const override { return detail; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_NPCWATCHMANSWEEPSPEED_HPP__
