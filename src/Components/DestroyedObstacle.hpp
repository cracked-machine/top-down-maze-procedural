#ifndef SRC_COMPONENTS_DESTROYEDOBSTACLE_HPP__
#define SRC_COMPONENTS_DESTROYEDOBSTACLE_HPP__

namespace Game::Cmp
{

//! @brief Component marking an obstacle entity as destroyed (e.g. by an armed bomb detonation).
struct DestroyedObstacle
{
  //! @brief Whether the destroyed state is currently active.
  bool active{ true };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_DESTROYEDOBSTACLE_HPP__
