#ifndef SRC_COMPONENTS_OBSTACLE_HPP__
#define SRC_COMPONENTS_OBSTACLE_HPP__

namespace Game::Cmp
{

// Mark the entity as a Rock obstacle
struct Obstacle
{
  bool m_enabled{ true };
  int damage{ 0 };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_OBSTACLE_HPP__
