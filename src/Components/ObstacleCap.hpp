#ifndef SRC_COMPONENTS_OBSTACLECAP_HPP__
#define SRC_COMPONENTS_OBSTACLECAP_HPP__

namespace Game::Cmp
{

//! @brief Marks the entity as the "cap" sprite rendered directly above its paired Obstacle entity
//! (tied together by a shared Cmp::UUID). Caps have no Obstacle component of their own.
struct ObstacleCap
{
  //! @brief Whether this cap is currently active/rendered.
  bool m_enabled{ true };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_OBSTACLECAP_HPP__
