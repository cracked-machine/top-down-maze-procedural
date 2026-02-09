#include <Components/AbsoluteAlpha.hpp>
#include <Components/Armable.hpp>
#include <Components/DestroyedObstacle.hpp>
#include <Components/Inventory/CarryItem.hpp>
#include <Components/Neighbours.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PlantObstacle.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <entt/entity/fwd.hpp>

namespace ProceduralMaze::Factory
{

void createObstacle( entt::registry &registry, entt::entity entity, Cmp::Position pos_cmp, Sprites::SpriteMetaType sprite_type,
                     std::size_t sprite_tile_idx, float zorder )

{
  if ( registry.all_of<Cmp::DestroyedObstacle>( entity ) ) { registry.remove<Cmp::DestroyedObstacle>( entity ); }
  registry.emplace_or_replace<Cmp::Obstacle>( entity );
  registry.emplace_or_replace<Cmp::ZOrderValue>( entity, pos_cmp.position.y + zorder );
  registry.emplace_or_replace<Cmp::NpcNoPathFinding>( entity );
  registry.emplace_or_replace<Cmp::AbsoluteAlpha>( entity, 255 );
  registry.emplace_or_replace<Cmp::SpriteAnimation>( entity, 0, 0, true, sprite_type, sprite_tile_idx );
  registry.emplace_or_replace<Cmp::Armable>( entity );
}

void destroyObstacle( entt::registry &registry, entt::entity obstacle_entity )
{
  if ( registry.all_of<Cmp::Obstacle>( obstacle_entity ) ) { registry.remove<Cmp::Obstacle>( obstacle_entity ); }
  if ( registry.all_of<Cmp::ZOrderValue>( obstacle_entity ) ) { registry.remove<Cmp::ZOrderValue>( obstacle_entity ); }
  if ( registry.all_of<Cmp::NpcNoPathFinding>( obstacle_entity ) ) { registry.remove<Cmp::NpcNoPathFinding>( obstacle_entity ); }
  if ( registry.all_of<Cmp::AbsoluteAlpha>( obstacle_entity ) ) { registry.remove<Cmp::AbsoluteAlpha>( obstacle_entity ); }
  if ( registry.all_of<Cmp::SpriteAnimation>( obstacle_entity ) ) { registry.remove<Cmp::SpriteAnimation>( obstacle_entity ); }

  registry.emplace_or_replace<Cmp::Armable>( obstacle_entity );
}

} // namespace ProceduralMaze::Factory
