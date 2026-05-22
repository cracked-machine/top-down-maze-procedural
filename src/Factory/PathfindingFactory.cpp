#include <Components/Position.hpp>
#include <Factory/PathfindingFactory.hpp>
#include <Npc/NpcNoPathFinding.hpp>
#include <PathFinding/SpatialHashGrid.hpp>
#include <SmartPointers.hpp>

namespace Game::Pathfinding::Factory
{

PathFinding::SpatialHashGridSharedPtr create_restricted_navmesh( entt::registry &reg )
{
  // create a restricted navmesh for pathfinding
  PathFinding::SpatialHashGridSharedPtr pathfinding_navmesh = std::make_shared<PathFinding::SpatialHashGrid>();
  for ( auto [pos_entt, pos_cmp] : reg.view<Cmp::Position>( entt::exclude<Cmp::NpcNoPathFinding> ).each() )
  {
    pathfinding_navmesh->insert( pos_entt, pos_cmp );
  }
  return pathfinding_navmesh;
}

PathFinding::SpatialHashGridSharedPtr create_open_navmesh( entt::registry &reg )
{
  // create a navmesh for uninhibited pathfinding
  PathFinding::SpatialHashGridSharedPtr open_navmesh = std::make_shared<PathFinding::SpatialHashGrid>();
  for ( auto [pos_entt, pos_cmp] : reg.view<Cmp::Position>().each() )
  {
    open_navmesh->insert( pos_entt, pos_cmp );
  }
  return open_navmesh;
}

} // namespace Game::Pathfinding::Factory