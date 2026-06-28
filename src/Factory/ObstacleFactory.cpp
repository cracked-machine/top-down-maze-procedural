#include <Components/AbsoluteAlpha.hpp>
#include <Components/AnimData.hpp>
#include <Components/Armable.hpp>
#include <Components/DestroyedObstacle.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PlantObstacle.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerNoPath.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/UUID.hpp>
#include <Components/VoidPosition.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Sprites/SpriteSheet.hpp>
#include <Utils/Constants.hpp>
#include <entt/entity/fwd.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace Game::Factory
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

void add_obstacle( entt::registry &reg, entt::entity entity )
{
  if ( reg.any_of<Cmp::PlayerCharacter, Cmp::ReservedPosition>( entity ) ) { return; }
  if ( reg.all_of<Cmp::DestroyedObstacle>( entity ) ) { reg.remove<Cmp::DestroyedObstacle>( entity ); }
  reg.emplace_or_replace<Cmp::Obstacle>( entity );
}

void decorate_obstacle( entt::registry &reg, entt::entity entity, Cmp::Position pos_cmp, const Sprites::SpriteSheet &ms, std::size_t sprite_tile_idx,
                        float zorder, bool blocking )
{
  if ( sprite_tile_idx > ms.get_sprite_count() - 1 )
  {
    throw std::runtime_error( "Unable to get index " + std::to_string( sprite_tile_idx ) + " in " + ms.get_sprite_type() +
                              " ( size: " + std::to_string( ms.get_sprite_count() ) + " )" );
  }
  Cmp::ZOrderValue zorder_cmp( 0 );

  // Use the non-zero function arg, or the non-zero json value, or fallback to the sprite y-axis
  if ( zorder != 0 ) { zorder_cmp.setZOrder( zorder ); }
  else if ( ms.get_zorder( sprite_tile_idx ) != 0 ) { zorder_cmp.setZOrder( ms.get_zorder( sprite_tile_idx ) ); }
  else { zorder_cmp.setZOrder( pos_cmp.position.y ); }

  if ( reg.any_of<Cmp::PlayerCharacter, Cmp::ReservedPosition>( entity ) ) { return; }
  if ( reg.all_of<Cmp::DestroyedObstacle>( entity ) ) { reg.remove<Cmp::DestroyedObstacle>( entity ); }
  reg.emplace_or_replace<Cmp::ZOrderValue>( entity, zorder );
  reg.emplace_or_replace<Cmp::NpcNoPathFinding>( entity );
  if ( blocking ) { reg.emplace_or_replace<Cmp::PlayerNoPath>( entity ); }
  reg.emplace_or_replace<Cmp::AbsoluteAlpha>( entity, 255 );
  reg.emplace_or_replace<Cmp::Armable>( entity );
  // clang-format off
  reg.emplace_or_replace<Cmp::AnimData>( entity, Cmp::AnimData::Config{ 
        .sprite_type = ms.get_sprite_type(), 
        .frame_index_offset = sprite_tile_idx,
        .enabled = true
  });
  // clang-format on
  reg.emplace_or_replace<Cmp::Armable>( entity );

  SPDLOG_DEBUG( "Added obstacle {} at [{},{}] Z: {}", ms.get_display_name(), pos_cmp.x(), pos_cmp.y(), zorder_cmp.getZOrder() );
}

void remove_obstacle( entt::registry &reg, entt::entity search_entt, bool delete_extras )
{
  if ( not reg.valid( search_entt ) ) return;

  Cmp::UUID search_uuid_cmp;
  auto *search_uuid_cmp_ptr = reg.try_get<Cmp::UUID>( search_entt );
  if ( search_uuid_cmp_ptr ) { search_uuid_cmp = *search_uuid_cmp_ptr; }

  reg.remove<Cmp::Obstacle>( search_entt );
  reg.remove<Cmp::ZOrderValue>( search_entt );
  reg.remove<Cmp::NpcNoPathFinding>( search_entt );
  reg.remove<Cmp::PlayerNoPath>( search_entt );
  reg.remove<Cmp::AbsoluteAlpha>( search_entt );
  reg.remove<Cmp::AnimData>( search_entt );
  reg.remove<Cmp::UUID>( search_entt );
  SPDLOG_DEBUG( "Removing obstacle {} - {}", static_cast<uint32_t>( search_entt ), search_uuid_cmp.str() );

  // don't search for all zeroes UUID
  if ( search_uuid_cmp.empty() )
  {
    SPDLOG_DEBUG( "This obstacle {} does not have a matching cap obstacle {}", static_cast<uint32_t>( search_entt ), search_uuid_cmp.str() );
    return;
  }

  // delete any cap entity (tied to obstacle entt by UUID)
  for ( auto [cap_entt, cap_uuid_cmp] : reg.view<Cmp::UUID>().each() )
  {
    if ( cap_uuid_cmp != search_uuid_cmp ) continue;
    SPDLOG_DEBUG( "Removing matching obstacle {} - {}", static_cast<uint32_t>( cap_entt ), search_uuid_cmp.str() );
    if ( delete_extras ) reg.destroy( cap_entt );
    break;
  }
}

void remove_obstacle( entt::registry &reg, entt::entity search_entt, bool delete_extras, const UUIDEntityMap &uuid_map )
{
  if ( not reg.valid( search_entt ) ) return;

  Cmp::UUID search_uuid_cmp;
  auto *search_uuid_cmp_ptr = reg.try_get<Cmp::UUID>( search_entt );
  if ( search_uuid_cmp_ptr ) { search_uuid_cmp = *search_uuid_cmp_ptr; }

  reg.remove<Cmp::Obstacle>( search_entt );
  reg.remove<Cmp::ZOrderValue>( search_entt );
  reg.remove<Cmp::NpcNoPathFinding>( search_entt );
  reg.remove<Cmp::PlayerNoPath>( search_entt );
  reg.remove<Cmp::AbsoluteAlpha>( search_entt );
  reg.remove<Cmp::AnimData>( search_entt );
  reg.remove<Cmp::UUID>( search_entt );
  SPDLOG_DEBUG( "Removing obstacle {} - {}", static_cast<uint32_t>( search_entt ), search_uuid_cmp.str() );

  if ( !delete_extras || search_uuid_cmp.empty() ) return;

  // O(1) cap entity lookup — map contains only cap entities (UUID present, no Obstacle)
  auto it = uuid_map.find( search_uuid_cmp );
  if ( it != uuid_map.end() && reg.valid( it->second ) )
  {
    SPDLOG_DEBUG( "Removing matching cap entity {} - {}", static_cast<uint32_t>( it->second ), search_uuid_cmp.str() );
    reg.destroy( it->second );
    return;
  }

  // Fallback: cap was created after the map was built (e.g. mid-shuffle decorate_interior_wall)
  for ( auto [cap_entt, cap_uuid_cmp] : reg.view<Cmp::UUID>().each() )
  {
    if ( cap_uuid_cmp != search_uuid_cmp ) continue;
    SPDLOG_DEBUG( "Removing cap entity (fallback scan) {} - {}", static_cast<uint32_t>( cap_entt ), search_uuid_cmp.str() );
    reg.destroy( cap_entt );
    break;
  }
}

} // namespace Game::Factory
