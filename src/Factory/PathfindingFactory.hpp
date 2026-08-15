#ifndef SRC_FACTORY_PATHFINDINGFACTORY_HPP__
#define SRC_FACTORY_PATHFINDINGFACTORY_HPP__

#include <PathFinding/SmartPointers.hpp>

#include <functional>

namespace Game::Factory::Pathfinding
{

//! @brief Decides whether a NpcNoPathFinding entity blocks a given navmesh variant.
//! Return false to let this navmesh's users pass through that blocker (e.g. ghosts
//! through plant segments, or future NPC types blocked only by specific plant types).
using NpcBlockerFilter = std::function<bool( entt::entity )>;

PathFinding::SpatialHashGridSharedPtr create_npc_navmesh( entt::registry &reg );
PathFinding::SpatialHashGridSharedPtr create_ghost_navmesh( entt::registry &reg );
PathFinding::SpatialHashGridSharedPtr create_player_navmesh( entt::registry &reg );
PathFinding::SpatialHashGridSharedPtr create_open_navmesh( entt::registry &reg );
PathFinding::SpatialHashGridSharedPtr create_reserved_navmesh( entt::registry &reg );

} // namespace Game::Factory::Pathfinding

#endif // SRC_FACTORY_PATHFINDINGFACTORY_HPP__
