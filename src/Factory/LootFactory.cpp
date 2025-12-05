#include <Components/Armable.hpp>
#include <Components/LootContainer.hpp>
#include <Components/ReservedPosition.hpp>
#include <Factory/LootFactory.hpp>

namespace ProceduralMaze::Factory
{

void createLootContainer( entt::registry &registry, entt::entity entt, Cmp::Position pos_cmp, Sprites::SpriteMetaType sprite_type,
                          std::size_t sprite_tile_idx, float zorder )
{

  registry.emplace_or_replace<Cmp::ReservedPosition>( entt );
  registry.emplace_or_replace<Cmp::Armable>( entt );
  registry.emplace_or_replace<Cmp::LootContainer>( entt );
  registry.emplace_or_replace<Cmp::SpriteAnimation>( entt, 0, 0, true, sprite_type, sprite_tile_idx );
  registry.emplace_or_replace<Cmp::ZOrderValue>( entt, pos_cmp.position.y - zorder );
}

void destroyLootContainer( entt::registry &registry, entt::entity loot_container_entity )
{
  registry.remove<Cmp::LootContainer>( loot_container_entity );
  registry.remove<Cmp::ReservedPosition>( loot_container_entity );
  registry.remove<Cmp::SpriteAnimation>( loot_container_entity );
  registry.remove<Cmp::ZOrderValue>( loot_container_entity );
}

void destroyLootDrop( entt::registry &registry, entt::entity loot_entity ) { registry.destroy( loot_entity ); }

} // namespace ProceduralMaze::Factory
