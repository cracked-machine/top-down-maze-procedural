#ifndef SRC_COMPONENTS_OBSTACLECAP_HPP__
#define SRC_COMPONENTS_OBSTACLECAP_HPP__

namespace Game::Cmp
{

// Marks the entity as the "cap" sprite rendered directly above its paired Obstacle entity
// (tied together by a shared Cmp::UUID). Caps have no Obstacle component of their own.
struct ObstacleCap
{
  bool m_enabled{ true };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_OBSTACLECAP_HPP__
