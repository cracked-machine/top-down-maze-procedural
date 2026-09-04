#include <Components/Altar/MultiBlock.hpp>
#include <Components/AnimData.hpp>
#include <Components/Crypt/BuildingMultiBlock.hpp>
#include <Components/Crypt/InteriorMultiBlock.hpp>
#include <Components/Grave/MultiBlock.hpp>
#include <Components/Grave/PlantSegment.hpp>
#include <Components/Moveable.hpp>
#include <Components/NoRender.hpp>
#include <Components/Npc/NoPathFinding.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/ObstacleCap.hpp>
#include <Components/Player/Character.hpp>
#include <Components/Player/NoPath.hpp>
#include <Components/Position.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/Ruin/BuildingMultiBlock.hpp>
#include <Components/Spring/HealingSpringBuildingMultiBlock.hpp>
#include <Components/Weapons/Arrow.hpp>
#include <Components/ZOrderValue.hpp>
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
  // instead of scanning every NoPath entity in the scene.
  PathFinding::SpatialHashGridSharedPtr player_navmesh = std::make_shared<PathFinding::SpatialHashGrid>();
  for ( auto [pos_entt, pos_cmp] : reg.view<Cmp::Player::NoPath, Cmp::Position>().each() )
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

void populate_render_position_grid( entt::registry &reg, PathFinding::SpatialHashGrid &render_position_grid )
{
  render_position_grid.clear();

  // Mirrors RenderGameSystem::add_visible_entity_to_z_order_queue()'s Cmp::Position fallback-path
  // exclude list exactly: movers are excluded here (and never inserted anywhere else) because
  // RenderGameSystem's add_mover_to_z_order_queue() scans them separately every frame instead; the 6
  // multiblock root types are excluded because they already get their own dedicated Z-order pass.
  auto static_render_view = reg.view<Cmp::Position, Cmp::AnimData, Cmp::ZOrderValue>(
      entt::exclude<Cmp::NoRender, Cmp::Player::Character, Cmp::Npc::NPC, Cmp::Weapons::Projectiles::Arrow, Cmp::Moveable, Cmp::ObstacleCap,
                    Cmp::Altar::MultiBlock, Cmp::Crypt::BuildingMultiBlock, Cmp::Grave::MultiBlock, Cmp::HealingSpringBuildingMultiBlock,
                    Cmp::Crypt::InteriorMultiBlock, Cmp::Ruin::BuildingMultiBlock> );
  for ( auto entity : static_render_view )
  {
    const auto &pos_cmp = static_render_view.get<Cmp::Position>( entity );
    render_position_grid.insert( entity, pos_cmp );
  }
}

PathFinding::SpatialHashGridSharedPtr create_render_position_grid( entt::registry &reg )
{
  PathFinding::SpatialHashGridSharedPtr render_position_grid = std::make_shared<PathFinding::SpatialHashGrid>();
  populate_render_position_grid( reg, *render_position_grid );
  return render_position_grid;
}

} // namespace Game::Factory::Pathfinding