#ifndef SRC_CMP_SCENESETTINGS_COLLISIONDETECTION_HPP__
#define SRC_CMP_SCENESETTINGS_COLLISIONDETECTION_HPP__

namespace Game::Cmp::SceneSettings
{

//! @brief Scene-level setting toggling collision detection, e.g. via the F1 debug/cheat key.
struct CollisionDetection
{
  //! @brief Whether collision detection is active.
  bool enabled{ true };
};

} // namespace Game::Cmp::SceneSettings

#endif // SRC_CMP_SCENESETTINGS_COLLISIONDETECTION_HPP__