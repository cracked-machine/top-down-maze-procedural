#ifndef SRC_COMPONENTS_HAZARD_COLLISIONRESIST_HPP__
#define SRC_COMPONENTS_HAZARD_COLLISIONRESIST_HPP__

namespace Game::Cmp::Hazard
{

//! @brief Attach to a lethal hazard cell (sinkhole/corruption/lava) so the player doesn't stumble
//!        into it on first contact. `PlayerSystem::is_valid_move` blocks entry like a wall until the
//!        player keeps pushing into the same cell for `resist_seconds`, then yields and allows the
//!        move through so the existing hazard collision checks can kill/damage the player as normal.
struct CollisionResist
{
  float resist_seconds{ 0.35f };
};

} // namespace Game::Cmp::Hazard

#endif // SRC_COMPONENTS_HAZARD_COLLISIONRESIST_HPP__
