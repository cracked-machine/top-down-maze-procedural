#ifndef SRC_FACTORY_PATHFINDINGFACTORY_HPP_
#define SRC_FACTORY_PATHFINDINGFACTORY_HPP_

#include <PathFinding/SmartPointers.hpp>

namespace ProceduralMaze::Pathfinding::Factory
{

PathFinding::SpatialHashGridSharedPtr create_restricted_navmesh( entt::registry &reg );
PathFinding::SpatialHashGridSharedPtr create_open_navmesh( entt::registry &reg );

} // namespace ProceduralMaze::Pathfinding::Factory

#endif // SRC_FACTORY_PATHFINDINGFACTORY_HPP_