#include <Components/Position.hpp>
#include <Components/Ruin/RuinBookcase.hpp>
#include <Components/Ruin/RuinCobweb.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/RuinFactory.hpp>
#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Factory
{

// void create_staircase( entt::registry &reg, sf::Vector2f spawn_position, const Sprites::MultiSprite &stairs_ms )
// {
//   auto stairs_entt = reg.create();
//   reg.emplace_or_replace<Cmp::Position>( stairs_entt, spawn_position, stairs_ms.getSpriteSizePixels() );
//   reg.emplace_or_replace<Cmp::SpriteAnimation>( stairs_entt, 0, 0, true, stairs_ms.get_sprite_type(), 0 );
//   reg.emplace_or_replace<Cmp::ZOrderValue>( stairs_entt, spawn_position.y );
// }

void create_bookcase( entt::registry &reg, sf::Vector2f spawn_position, const Sprites::MultiSprite &bookcase_ms, int sprite_index )
{
  auto bookcase_entt = reg.create();
  reg.emplace_or_replace<Cmp::Position>( bookcase_entt, spawn_position, bookcase_ms.getSpriteSizePixels() );
  reg.emplace_or_replace<Cmp::SpriteAnimation>( bookcase_entt, 0, 0, true, bookcase_ms.get_sprite_type(), sprite_index );
  reg.emplace_or_replace<Cmp::ZOrderValue>( bookcase_entt, -spawn_position.y * 10 );
  reg.emplace_or_replace<Cmp::RuinBookcase>( bookcase_entt );
}

void create_cobweb( entt::registry &reg, sf::Vector2f spawn_position, const Sprites::MultiSprite &cobweb_ms, int sprite_index )
{
  auto cobweb_entt = reg.create();
  reg.emplace_or_replace<Cmp::Position>( cobweb_entt, spawn_position, cobweb_ms.getSpriteSizePixels() );
  reg.emplace_or_replace<Cmp::SpriteAnimation>( cobweb_entt, 0, 0, true, cobweb_ms.get_sprite_type(), sprite_index );
  reg.emplace_or_replace<Cmp::ZOrderValue>( cobweb_entt, spawn_position.y );
  reg.emplace_or_replace<Cmp::RuinCobweb>( cobweb_entt, 100 );
}

} // namespace ProceduralMaze::Factory