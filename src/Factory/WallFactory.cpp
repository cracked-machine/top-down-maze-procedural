#include <Components/AnimData.hpp>
#include <Components/NoMoveDest.hpp>
#include <Components/Npc/NoPathFinding.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerNoPath.hpp>
#include <Components/Position.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/Wall.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/WallFactory.hpp>
#include <Sprites/SpriteSheet.hpp>
#include <Utils/Constants.hpp>

#include <entt/entity/registry.hpp>

namespace Game::Factory::Wall
{

void add_wall_entity( entt::registry &reg, const sf::Vector2f &pos, const Sprites::SpriteSheet &ms, std::size_t sprite_index )
{
  auto entity = reg.create();
  reg.emplace_or_replace<Cmp::Position>( entity, pos, Constants::kGridSizePxF );
  reg.emplace_or_replace<Cmp::Wall>( entity );
  reg.emplace_or_replace<Cmp::ReservedPosition>( entity );
  // clang-format off
  reg.emplace_or_replace<Cmp::AnimData>( entity, Cmp::AnimData::Config{ 
      .sprite_type = ms.get_sprite_type(), 
      .frame_index_offset = static_cast<size_t>(sprite_index),
      .enabled = true
  });
  // clang-format on
  Cmp::ZOrderValue zorder_cmp( 0 );
  if ( ms.get_zorder( sprite_index ) != 0 ) { zorder_cmp.setZOrder( ms.get_zorder( sprite_index ) ); }
  else { zorder_cmp.setZOrder( pos.y + ms.get_sprite_size().y ); }
  reg.emplace_or_replace<Cmp::ZOrderValue>( entity, zorder_cmp );
}

void add_reservedposition( entt::registry &reg, const sf::Vector2f &pos )
{
  auto entity = reg.create();
  reg.emplace_or_replace<Cmp::Position>( entity, pos, Constants::kGridSizePxF );
  reg.emplace_or_replace<Cmp::ReservedPosition>( entity );
}

void add_solid_player( entt::registry &reg, sf::FloatRect rect )
{
  // Mark any existing world position entities overlapping this rect as reserved.
  // Don't mark player position as Cmp::PlayerNoPath or they won't be able to move!
  for ( auto [entt, pos_cmp] : reg.view<Cmp::Position>( entt::exclude<Cmp::PlayerCharacter> ).each() )
  {
    if ( pos_cmp.findIntersection( rect ) )
    {
      reg.emplace_or_replace<Cmp::ReservedPosition>( entt );
      reg.emplace_or_replace<Cmp::PlayerNoPath>( entt );
    }
  }
}

void add_solid_npc( entt::registry &reg, sf::FloatRect rect )
{
  // Mark any existing world position entities overlapping this rect as reserved
  for ( auto [entt, pos_cmp] : reg.view<Cmp::Position>().each() )
  {
    if ( pos_cmp.findIntersection( rect ) )
    {
      reg.emplace_or_replace<Cmp::ReservedPosition>( entt );
      reg.emplace_or_replace<Cmp::Npc::NoPathFinding>( entt );
    }
  }
}

void add_no_move_dest( entt::registry &reg, sf::FloatRect rect )
{
  // Mark any existing world position entities overlapping this rect as reserved
  for ( auto [entt, pos_cmp] : reg.view<Cmp::Position>().each() )
  {
    if ( pos_cmp.findIntersection( rect ) )
    {
      reg.emplace_or_replace<Cmp::ReservedPosition>( entt );
      reg.emplace_or_replace<Cmp::NoMoveDest>( entt );
    }
  }
}

} // namespace Game::Factory::Wall