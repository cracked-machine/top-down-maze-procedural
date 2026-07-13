#ifndef SRC_FACTORY_PATHFINDINGFACTORY_HPP__
#define SRC_FACTORY_PATHFINDINGFACTORY_HPP__

#include <PathFinding/SmartPointers.hpp>

namespace Game::Pathfinding::Factory
{

PathFinding::SpatialHashGridSharedPtr create_npc_navmesh( entt::registry &reg );
PathFinding::SpatialHashGridSharedPtr create_player_navmesh( entt::registry &reg );
PathFinding::SpatialHashGridSharedPtr create_open_navmesh( entt::registry &reg );
PathFinding::SpatialHashGridSharedPtr create_reserved_navmesh( entt::registry &reg );

} // namespace Game::Pathfinding::Factory

#endif // SRC_FACTORY_PATHFINDINGFACTORY_HPP__
