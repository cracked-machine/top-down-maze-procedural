#include <Components/Armable.hpp>
#include <Components/LootContainer.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/ReservedPosition.hpp>
#include <Factory/LootFactory.hpp>
#include <SpatialHashGrid.hpp>
#include <Sprites/SpriteSheet.hpp>
#include <Utils/Random.hpp>

namespace Game::Factory
{

void create_loot_container( entt::registry &reg, entt::entity entt, Cmp::Position pos_cmp, Sprites::SpriteMetaType sprite_type,
                            std::size_t sprite_tile_idx, float zorder )
{

  reg.emplace_or_replace<Cmp::ReservedPosition>( entt );
  reg.emplace_or_replace<Cmp::Armable>( entt );
  reg.emplace_or_replace<Cmp::LootContainer>( entt );
  // clang-format off
  reg.emplace_or_replace<Cmp::AnimData>( entt, Cmp::AnimData::Config{ 
        .sprite_type = sprite_type, 
        .frame_index_offset = sprite_tile_idx,
        .framerate = 0.2,
        .enabled = false,
        .anim_type =   Cmp::AnimType::ONESHOTRESET
  });
  // clang-format on 
  reg.emplace_or_replace<Cmp::ZOrderValue>( entt, pos_cmp.position.y - zorder );
}

void destroy_loot_container( entt::registry &registry, entt::entity loot_container_entity )
{
  registry.remove<Cmp::LootContainer>( loot_container_entity );
  registry.remove<Cmp::ReservedPosition>( loot_container_entity );
  registry.remove<Cmp::AnimData>( loot_container_entity );
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

    float zorder = sprite_factory.get_sprite_size_by_type( "sprite.graveyard.pots" ).y;

    Cmp::RandomInt pot_picker( 0, 2 );
    Factory::create_loot_container( reg, random_entity, random_origin_position, "sprite.graveyard.pots", pot_picker.gen(), zorder );
    assigned_entts.push_back( random_entity );
  }

  return assigned_entts;
}

} // namespace Game::Factory
