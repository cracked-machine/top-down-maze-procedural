#ifndef SRC_COMPONENTS_NPC_TARGET_HPP__
#define SRC_COMPONENTS_NPC_TARGET_HPP__

#include <Components/Position.hpp>

namespace Game::Cmp::Npc
{
//! @brief Used for NPC pathfinding if their target is not the player.
// Add this component to an entity that already has a Cmp::Position to mark it as a NPC target
struct Target
{
  Target( entt::entity id )
      : id( id )
  {
  }
  entt::entity id{ entt::null };
};

} // namespace Game::Cmp::Npc

#endif // SRC_COMPONENTS_NPC_TARGET_HPP__
