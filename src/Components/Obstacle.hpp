#ifndef SRC_COMPONENTS_OBSTACLE_HPP__
#define SRC_COMPONENTS_OBSTACLE_HPP__

namespace Game::Cmp
{

//! @brief Mark the entity as a Rock obstacle
struct Obstacle
{
  //! @brief Whether this obstacle is currently active/blocking.
  bool m_enabled{ true };

  //! @brief Damage dealt by this obstacle (e.g. on collision).
  int damage{ 0 };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_OBSTACLE_HPP__
