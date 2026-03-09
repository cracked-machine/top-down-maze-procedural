#include <Components/Armable.hpp>
#include <Components/LootContainer.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/ReservedPosition.hpp>
#include <Factory/LootFactory.hpp>
#include <SpatialHashGrid.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Utils/Random.hpp>

namespace ProceduralMaze::Factory
{

void create_loot_container( entt::registry &registry, entt::entity entt, Cmp::Position pos_cmp, Sprites::SpriteMetaType sprite_type,
                            std::size_t sprite_tile_idx, float zorder )
{

  registry.emplace_or_replace<Cmp::ReservedPosition>( entt );
  registry.emplace_or_replace<Cmp::Armable>( entt );
  registry.emplace_or_replace<Cmp::LootContainer>( entt );
  registry.emplace_or_replace<Cmp::SpriteAnimation>( entt, 0, 0, false, sprite_type, sprite_tile_idx );
  registry.emplace_or_replace<Cmp::ZOrderValue>( entt, pos_cmp.position.y - zorder );
}

void destroy_loot_container( entt::registry &registry, entt::entity loot_container_entity )
{
  registry.remove<Cmp::LootContainer>( loot_container_entity );
  registry.remove<Cmp::ReservedPosition>( loot_container_entity );
  registry.remove<Cmp::SpriteAnimation>( loot_container_entity );
  registry.remove<Cmp::ZOrderValue>( loot_container_entity );
}

void destroy_loot_drop( entt::registry &registry, entt::entity loot_entity ) { registry.destroy( loot_entity ); }

std::vector<entt::entity> gen_loot_containers( entt::registry &reg, Sprites::SpriteFactory &sprite_factory, sf::Vector2u map_grid_size )
{
  std::vector<entt::entity> assigned_entts;

  auto num_loot_containers = map_grid_size.x * map_grid_size.y / 120; // one loot container per N grid squares

  for ( std::size_t i = 0; i < num_loot_containers; ++i )
  {
    auto [random_entity, random_origin_position] = Utils::Rnd::get_random_position(
        reg, {}, Utils::Rnd::ExcludePack<Cmp::PlayerCharacter, Cmp::ReservedPosition, Cmp::Obstacle>{}, 0 );

    float zorder = sprite_factory.get_sprite_size_by_type( "POT" ).y;

    Factory::create_loot_container( reg, random_entity, random_origin_position, "POT", 0, zorder );
    assigned_entts.push_back( random_entity );
  }

  return assigned_entts;
}

} // namespace ProceduralMaze::Factory
