#ifndef SRC_CMP_SCENESETTINGS_SHOWDEBUGSTATS_HPP__
#define SRC_CMP_SCENESETTINGS_SHOWDEBUGSTATS_HPP__

namespace Game::Cmp::SceneSettings
{

//! @brief Scene-level setting toggling the on-screen debug stats overlay, e.g. via the F3 debug/cheat key.
struct ShowDebugStats
{
  //! @brief Whether the debug stats overlay is shown.
  bool enabled{ false };
};

} // namespace Game::Cmp::SceneSettings

#endif // SRC_CMP_SCENESETTINGS_SHOWDEBUGSTATS_HPP__