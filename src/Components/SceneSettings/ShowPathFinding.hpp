#ifndef SRC_CMP_SCENESETTINGS_SHOWPATHFINDING_HPP__
#define SRC_CMP_SCENESETTINGS_SHOWPATHFINDING_HPP__

namespace Game::Cmp::SceneSettings
{

//! @brief Scene-level setting toggling the debug pathfinding overlay, e.g. via the F2 debug/cheat key.
struct ShowPathFinding
{
  //! @brief Whether the pathfinding overlay is shown.
  bool enabled{ false };
};

} // namespace Game::Cmp::SceneSettings

#endif // SRC_CMP_SCENESETTINGS_SHOWPATHFINDING_HPP__