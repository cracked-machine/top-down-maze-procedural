#ifndef SRC_UTILS_NPC_HPP__
#define SRC_UTILS_NPC_HPP__

#include <Sprites/SpriteMetaType.hpp>
#include <source_location>
namespace Game::Cmp
{
class Position;
}

namespace Game::PathFinding
{
class SpatialHashGrid;
}

namespace Game::Utils::Npc
{

entt::entity get_world_pos_entt( entt::registry &reg, Cmp::Position npc_pos );
entt::entity get_world_pos_entt( entt::registry &reg, entt::entity npc_entt );

Sprites::SpriteMetaType get_sprite_type( entt::registry &reg, entt::entity npc_entt, std::source_location loc = std::source_location::current() );

//! @brief Outcome of a single pathfind_toward() step
enum class PathfindResult {
  Moved,   //! a path was found and the NPC's Direction/LerpPosition were updated
  Blocked, //! the NPC is off-screen, mid-lerp, or would cross the spawn boundary — no A* was run or the result was discarded
  NoPath,  //! A* found no usable path; caller decides how to react (e.g. zero direction, pick a new target)
};

//! @brief Pathfind a single NPC entity one grid step towards `target_pos` and, on success, update its
//! Cmp::Direction/Cmp::LerpPosition. Shared by NpcSystem (player-chasing NPCs) and WispSystem (target-chasing wisps).
//! @param reg
//! @param navmesh
//! @param target_pos
//! @param npc_entity
//! @param target_in_spawn true if `target_pos` is within the spawn area, used to stop the NPC at the boundary
//! @param always_pathfind skip the "only pathfind when on-screen" check — wisps must always pathfind
PathfindResult pathfind_toward( entt::registry &reg, PathFinding::SpatialHashGrid &navmesh, const Cmp::Position &target_pos, entt::entity npc_entity,
                                bool target_in_spawn, bool always_pathfind = false );

} // namespace Game::Utils::Npc

#endif // SRC_UTILS_NPC_HPP__
