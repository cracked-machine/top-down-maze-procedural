#ifndef SRC_COMPONENTS_NPC_NPCTARGET_HPP__
#define SRC_COMPONENTS_NPC_NPCTARGET_HPP__

#include <Components/Position.hpp>

namespace Game::Cmp
{
//! @brief Used for NPC pathfinding if their target is not the player.
// Add this component to an entity that already has a Cmp::Position to mark it as a NPC target
struct NpcTarget
{
  NpcTarget( entt::entity id )
      : id( id )
  {
  }
  entt::entity id{ entt::null };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_NPC_NPCTARGET_HPP__
