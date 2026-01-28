#include <Components/Position.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/RuinFactory.hpp>
#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Factory
{

void create_staircase( entt::registry &reg, sf::Vector2f spawn_position, const Sprites::MultiSprite &stairs_ms )
{
  auto stairs_entt = reg.create();
  reg.emplace_or_replace<Cmp::Position>( stairs_entt, spawn_position, stairs_ms.getSpriteSizePixels() );
  reg.emplace_or_replace<Cmp::SpriteAnimation>( stairs_entt, 0, 0, true, stairs_ms.get_sprite_type(), 0 );
  reg.emplace_or_replace<Cmp::ZOrderValue>( stairs_entt, spawn_position.y );
}

} // namespace ProceduralMaze::Factory