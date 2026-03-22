#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Player/PlayerNoPath.hpp>
#include <Components/Position.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/Wall.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/WallFactory.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Utils/Constants.hpp>
#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Factory
{

void add_wall_entity( entt::registry &reg, const sf::Vector2f &pos, const Sprites::MultiSprite &ms, std::size_t sprite_index, SolidWall solid_wall )
{
  auto entity = reg.create();
  reg.emplace_or_replace<Cmp::Position>( entity, pos, Constants::kGridSizePxF );
  reg.emplace_or_replace<Cmp::Wall>( entity, static_cast<bool>( solid_wall ) );
  reg.emplace_or_replace<Cmp::ReservedPosition>( entity );
  reg.emplace_or_replace<Cmp::SpriteAnimation>( entity, 0, 0, true, ms.get_sprite_type(), sprite_index );

  if ( solid_wall == SolidWall::TRUE )
  {
    reg.emplace_or_replace<Cmp::PlayerNoPath>( entity );
    reg.emplace_or_replace<Cmp::NpcNoPathFinding>( entity );
  }

  Cmp::ZOrderValue zorder_cmp( 0 );
  if ( ms.get_zorder( sprite_index ) != 0 ) { zorder_cmp.setZOrder( ms.get_zorder( sprite_index ) ); }
  else { zorder_cmp.setZOrder( pos.y + ms.getSpriteSizePixels().y ); }
  reg.emplace_or_replace<Cmp::ZOrderValue>( entity, zorder_cmp );
}

void add_nopathfinding( entt::registry &reg, const sf::Vector2f &pos )
{
  auto entity = reg.create();
  reg.emplace_or_replace<Cmp::Position>( entity, pos, Constants::kGridSizePxF );
  reg.emplace_or_replace<Cmp::PlayerNoPath>( entity );
}

void add_nonpcpathfinding( entt::registry &reg, const sf::Vector2f &pos )
{
  auto entity = reg.create();
  reg.emplace_or_replace<Cmp::Position>( entity, pos, Constants::kGridSizePxF );
  reg.emplace_or_replace<Cmp::NpcNoPathFinding>( entity );
}

void add_reservedposition( entt::registry &reg, const sf::Vector2f &pos )
{
  auto entity = reg.create();
  reg.emplace_or_replace<Cmp::Position>( entity, pos, Constants::kGridSizePxF );
  reg.emplace_or_replace<Cmp::ReservedPosition>( entity );
}

} // namespace ProceduralMaze::Factory