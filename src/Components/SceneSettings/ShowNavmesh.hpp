#ifndef SRC_CMP_SCENESETTINGS_SHOWNAVMESH_HPP__
#define SRC_CMP_SCENESETTINGS_SHOWNAVMESH_HPP__

namespace Game::Cmp::SceneSettings
{

//! @brief Scene-level setting toggling the debug navmesh overlay, e.g. via the F4 debug/cheat key.
struct ShowNavmesh
{
  //! @brief Whether the navmesh overlay is shown.
  bool enabled{ false };
};

} // namespace Game::Cmp::SceneSettings

#endif // SRC_CMP_SCENESETTINGS_SHOWNAVMESH_HPP__