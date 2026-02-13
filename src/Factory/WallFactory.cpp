#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Player/PlayerNoPath.hpp>
#include <Components/Position.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/Wall.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/WallFactory.hpp>
#include <Utils/Constants.hpp>
#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Factory
{

void add_wall_entity( entt::registry &reg, const sf::Vector2f &pos, Sprites::SpriteMetaType sprite_type, std::size_t sprite_index, float zorder,
                      SolidWall solid_wall )
{
  auto entity = reg.create();
  reg.emplace_or_replace<Cmp::Position>( entity, pos, Constants::kGridSquareSizePixelsF );
  reg.emplace_or_replace<Cmp::Wall>( entity, static_cast<bool>( solid_wall ) );
  // reg.emplace_or_replace<Cmp::Obstacle>( entity );
  reg.emplace_or_replace<Cmp::SpriteAnimation>( entity, 0, 0, true, sprite_type, sprite_index );
  reg.emplace_or_replace<Cmp::ReservedPosition>( entity );
  reg.emplace_or_replace<Cmp::ZOrderValue>( entity, zorder );
}

void add_nopathfinding( entt::registry &reg, const sf::Vector2f &pos )
{
  auto entity = reg.create();
  reg.emplace_or_replace<Cmp::Position>( entity, pos, Constants::kGridSquareSizePixelsF );
  reg.emplace_or_replace<Cmp::PlayerNoPath>( entity );
}

} // namespace ProceduralMaze::Factory