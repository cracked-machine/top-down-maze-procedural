#include <Components/AbsoluteAlpha.hpp>
#include <Components/Armable.hpp>
#include <Components/DestroyedObstacle.hpp>
#include <Components/Inventory/CarryItem.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PlantObstacle.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/ZOrderValue.hpp>
#include <Constants.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Player/PlayerCharacter.hpp>
#include <Player/PlayerNoPath.hpp>
#include <VoidPosition.hpp>
#include <entt/entity/fwd.hpp>

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

void create_obstacle( entt::registry &registry, entt::entity entity, Cmp::Position pos_cmp, Sprites::SpriteMetaType sprite_type,
                      std::size_t sprite_tile_idx, float zorder )

{
  if ( registry.all_of<Cmp::PlayerCharacter>( entity ) ) { return; }
  if ( registry.all_of<Cmp::DestroyedObstacle>( entity ) ) { registry.remove<Cmp::DestroyedObstacle>( entity ); }
  registry.emplace_or_replace<Cmp::Obstacle>( entity );
  registry.emplace_or_replace<Cmp::ZOrderValue>( entity, pos_cmp.position.y + zorder );
  registry.emplace_or_replace<Cmp::NpcNoPathFinding>( entity );
  registry.emplace_or_replace<Cmp::PlayerNoPath>( entity );
  registry.emplace_or_replace<Cmp::AbsoluteAlpha>( entity, 255 );
  registry.emplace_or_replace<Cmp::SpriteAnimation>( entity, 0, 0, true, sprite_type, sprite_tile_idx );
  registry.emplace_or_replace<Cmp::Armable>( entity );
}

void remove_obstacle( entt::registry &reg, entt::entity entt )
{
  if ( reg.all_of<Cmp::Obstacle>( entt ) ) { reg.remove<Cmp::Obstacle>( entt ); }
  if ( reg.all_of<Cmp::ZOrderValue>( entt ) ) { reg.remove<Cmp::ZOrderValue>( entt ); }
  if ( reg.all_of<Cmp::NpcNoPathFinding>( entt ) ) { reg.remove<Cmp::NpcNoPathFinding>( entt ); }
  if ( reg.all_of<Cmp::PlayerNoPath>( entt ) ) { reg.remove<Cmp::PlayerNoPath>( entt ); }
  if ( reg.all_of<Cmp::AbsoluteAlpha>( entt ) ) { reg.remove<Cmp::AbsoluteAlpha>( entt ); }
  if ( reg.all_of<Cmp::SpriteAnimation>( entt ) ) { reg.remove<Cmp::SpriteAnimation>( entt ); }

  reg.emplace_or_replace<Cmp::Armable>( entt );
}

} // namespace ProceduralMaze::Factory
