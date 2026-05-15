#ifndef SRC_CMPS_NPC_NPCTARGET_HPP_
#define SRC_CMPS_NPC_NPCTARGET_HPP_

#include <Components/Position.hpp>

namespace ProceduralMaze::Cmp
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

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_NPC_NPCTARGET_HPP_