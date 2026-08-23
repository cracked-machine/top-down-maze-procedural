#ifndef SRC_CMP_SCENESETTINGS_SHADERS_HPP__
#define SRC_CMP_SCENESETTINGS_SHADERS_HPP__

namespace Game::Cmp::SceneSettings
{

//! @brief Scene-level setting toggling shader effects, e.g. via the F9 debug/cheat key.
struct Shaders
{
  //! @brief Whether shader effects are active.
  bool enabled{ true };
};

} // namespace Game::Cmp::SceneSettings

#endif // SRC_CMP_SCENESETTINGS_SHADERS_HPP__