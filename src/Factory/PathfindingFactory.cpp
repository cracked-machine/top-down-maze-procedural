#include <Components/Grave/PlantSegment.hpp>
#include <Components/Npc/NoPathFinding.hpp>
#include <Components/Player/PlayerNoPath.hpp>
#include <Components/Position.hpp>
#include <Components/ReservedPosition.hpp>
#include <Factory/PathfindingFactory.hpp>
#include <PathFinding/SmartPointers.hpp>
#include <PathFinding/SpatialHashGrid.hpp>

namespace Game::Factory::Pathfinding
{

PathFinding::SpatialHashGridSharedPtr create_npc_navmesh( entt::registry &reg )
{

  // first pass: find all navigable positions
  PathFinding::SpatialHashGridSharedPtr pathfinding_navmesh = std::make_shared<PathFinding::SpatialHashGrid>();
  for ( auto [pos_entt, pos_cmp] : reg.view<Cmp::Position>().each() )
  {
    pathfinding_navmesh->insert( pos_entt, pos_cmp );
  }
  // second pass: If we find any position with a Cmp::Npc::NoPathFinding then that invalidates the entire bucket at that position.
  for ( auto [pos_entt, pos_cmp] : reg.view<Cmp::Position>().each() )
  {
    if ( reg.all_of<Cmp::Npc::NoPathFinding>( pos_entt ) ) { pathfinding_navmesh->remove_all( pos_cmp ); }
  }
  return pathfinding_navmesh;
}

PathFinding::SpatialHashGridSharedPtr create_ghost_navmesh( entt::registry &reg )
{
  // first pass: find all navigable positions
  PathFinding::SpatialHashGridSharedPtr pathfinding_navmesh = std::make_shared<PathFinding::SpatialHashGrid>();
  for ( auto [pos_entt, pos_cmp] : reg.view<Cmp::Position>().each() )
  {
    pathfinding_navmesh->insert( pos_entt, pos_cmp );
  }
  // second pass: If we find any position with a Cmp::Npc::NoPathFinding then that invalidates the entire bucket at that position.
  for ( auto [pos_entt, pos_cmp] : reg.view<Cmp::Position>().each() )
  {
    if ( reg.all_of<Cmp::Npc::NoPathFinding>( pos_entt ) ) { pathfinding_navmesh->remove_all( pos_cmp ); }
  }
  return pathfinding_navmesh;
}

PathFinding::SpatialHashGridSharedPtr create_player_navmesh( entt::registry &reg )
{
  // Index only the blocking obstacles so is_valid_move can do a spatial lookup
  // instead of scanning every PlayerNoPath entity in the scene.
  PathFinding::SpatialHashGridSharedPtr player_navmesh = std::make_shared<PathFinding::SpatialHashGrid>();
  for ( auto [pos_entt, nopath_cmp, pos_cmp] : reg.view<Cmp::PlayerNoPath, Cmp::Position>().each() )
  {
    player_navmesh->insert( pos_entt, pos_cmp );
  }
  return player_navmesh;
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

PathFinding::SpatialHashGridSharedPtr create_reserved_navmesh( entt::registry &reg )
{
  // create a navmesh for uninhibited pathfinding
  PathFinding::SpatialHashGridSharedPtr reserved_navmesh = std::make_shared<PathFinding::SpatialHashGrid>();
  for ( auto [pos_entt, reserved_cmp, pos_cmp] : reg.view<Cmp::ReservedPosition, Cmp::Position>().each() )
  {
    reserved_navmesh->insert( pos_entt, pos_cmp );
  }
  return reserved_navmesh;
}

} // namespace Game::Factory::Pathfinding