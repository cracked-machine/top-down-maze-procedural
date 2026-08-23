#ifndef SRC_CMP_SCENESETTINGS_FOOTSTEPS_HPP__
#define SRC_CMP_SCENESETTINGS_FOOTSTEPS_HPP__

namespace Game::Cmp::SceneSettings
{

//! @brief Scene-level setting toggling footstep effects (e.g. sound/visuals), e.g. via the F10 debug/cheat key.
struct Footsteps
{
  //! @brief Whether footstep effects are active.
  bool enabled{ true };
};

} // namespace Game::Cmp::SceneSettings

#endif // SRC_CMP_SCENESETTINGS_FOOTSTEPS_HPP__