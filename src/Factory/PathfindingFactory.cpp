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

PathFinding::SpatialHashGridSharedPtr create_npc_navmesh( entt::registry &reg, const NpcBlockerFilter &blocks )
{
  // Blocking components can live on a separate entity (e.g. a multiblock segment) that
  // shares a grid position with a plain world tile, so excluding by entity alone would
  // still insert the tile. Collect the blocked positions first, then skip any entity
  // whose position is blocked. Only structural entities (ReservedPosition) block their
  // position: decorations like obstacle caps also carry NpcNoPathFinding, but they only
  // exclude themselves - the tile underneath them must stay walkable.
  PathFinding::SpatialHashGrid blocked_positions;
  for ( auto [nopath_entt, nopath_cmp, reserved_cmp, pos_cmp] : reg.view<Cmp::Npc::NoPathFinding, Cmp::ReservedPosition, Cmp::Position>().each() )
  {
    if ( blocks && not blocks( nopath_entt ) ) continue;
    blocked_positions.insert( nopath_entt, pos_cmp );
  }

  // create a restricted navmesh for pathfinding
  PathFinding::SpatialHashGridSharedPtr pathfinding_navmesh = std::make_shared<PathFinding::SpatialHashGrid>();
  for ( auto [pos_entt, pos_cmp] : reg.view<Cmp::Position>( entt::exclude<Cmp::Npc::NoPathFinding> ).each() )
  {
    if ( not blocked_positions.at( pos_cmp ).empty() ) continue;
    pathfinding_navmesh->insert( pos_entt, pos_cmp );
  }
  return pathfinding_navmesh;
}

PathFinding::SpatialHashGridSharedPtr create_ghost_navmesh( entt::registry &reg )
{
  // Ghosts drift through plants but respect every other solid blocker. Further NPC-type
  // specific navmeshes (e.g. blocked only by certain plant types) follow this pattern:
  // pass a filter that inspects the blocker entity's components.
  return create_npc_navmesh( reg, [&reg]( entt::entity blocker_entt ) { return not reg.any_of<Cmp::PlantSegment>( blocker_entt ); } );
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