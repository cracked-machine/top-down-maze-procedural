#include <Components/AbsoluteAlpha.hpp>
#include <Components/AnimData.hpp>
#include <Components/Armable.hpp>
#include <Components/DestroyedObstacle.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PlantObstacle.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/ZOrderValue.hpp>
#include <Constants.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Player/PlayerCharacter.hpp>
#include <Player/PlayerNoPath.hpp>
#include <Sprites/SpriteSheet.hpp>
#include <VoidPosition.hpp>
#include <entt/entity/fwd.hpp>
#include <stdexcept>

namespace ProceduralMaze::Factory
{

entt::entity create_world_pos( entt::registry &registry, const sf::Vector2f &pos )
{
  auto entity = registry.create();
  registry.emplace_or_replace<Cmp::Position>( entity, pos, Constants::kGridSizePxF );
  return entity;
}

entt::entity create_void_pos( entt::registry &registry, const Cmp::Position &pos )
{
  auto entity = registry.create();
  registry.emplace_or_replace<Cmp::VoidPosition>( entity, pos );
  return entity;
}

void create_obstacle( entt::registry &reg, entt::entity entity, Cmp::Position pos_cmp, const Sprites::SpriteSheet &ms, std::size_t sprite_tile_idx )
{
  if ( sprite_tile_idx > ms.get_sprite_count() - 1 )
  {
    throw std::runtime_error( "Unable to get index " + std::to_string( sprite_tile_idx ) + " in " + ms.get_sprite_type() +
                              " ( size: " + std::to_string( ms.get_sprite_count() ) + " )" );
  }
  Cmp::ZOrderValue zorder( 0 );

  for ( auto [plant_entt, plant_cmp, plant_pos_cmp] : reg.view<Cmp::PlantObstacle, Cmp::Position>().each() )
  {
    if ( pos_cmp.findIntersection( plant_pos_cmp ) ) return;
  }

  if ( ms.get_zorder( sprite_tile_idx ) != 0 ) { zorder.setZOrder( ms.get_zorder( sprite_tile_idx ) ); }
  else { zorder.setZOrder( pos_cmp.position.y ); }

  if ( reg.any_of<Cmp::PlayerCharacter, Cmp::ReservedPosition>( entity ) ) { return; }
  if ( reg.all_of<Cmp::DestroyedObstacle>( entity ) ) { reg.remove<Cmp::DestroyedObstacle>( entity ); }
  reg.emplace_or_replace<Cmp::Obstacle>( entity );
  reg.emplace_or_replace<Cmp::ZOrderValue>( entity, zorder );
  reg.emplace_or_replace<Cmp::NpcNoPathFinding>( entity );
  reg.emplace_or_replace<Cmp::PlayerNoPath>( entity );
  reg.emplace_or_replace<Cmp::AbsoluteAlpha>( entity, 255 );
  // clang-format off
  reg.emplace_or_replace<Cmp::AnimData>( entity, Cmp::AnimData::Config{ 
        .sprite_type = ms.get_sprite_type(), 
        .frame_index_offset = sprite_tile_idx,
        .enabled = true
  });
  // clang-format on 
  reg.emplace_or_replace<Cmp::Armable>( entity );
}

void remove_obstacle( entt::registry &reg, entt::entity entt )
{
  if ( reg.all_of<Cmp::Obstacle>( entt ) ) { reg.remove<Cmp::Obstacle>( entt ); }
  if ( reg.all_of<Cmp::ZOrderValue>( entt ) ) { reg.remove<Cmp::ZOrderValue>( entt ); }
  if ( reg.all_of<Cmp::NpcNoPathFinding>( entt ) ) { reg.remove<Cmp::NpcNoPathFinding>( entt ); }
  if ( reg.all_of<Cmp::PlayerNoPath>( entt ) ) { reg.remove<Cmp::PlayerNoPath>( entt ); }
  if ( reg.all_of<Cmp::AbsoluteAlpha>( entt ) ) { reg.remove<Cmp::AbsoluteAlpha>( entt ); }
  if ( reg.all_of<Cmp::AnimData>( entt ) ) { reg.remove<Cmp::AnimData>( entt ); }

  reg.emplace_or_replace<Cmp::Armable>( entt );
}

} // namespace ProceduralMaze::Factory
