#include <Components/AbsoluteAlpha.hpp>
#include <Components/Armable.hpp>
#include <Components/Inventory/CarryItem.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/PlantObstacle.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/PlantFactory.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Utils/Random.hpp>

namespace ProceduralMaze::Factory
{

entt::entity createPlantObstacle( entt::registry &reg, Cmp::Position pos_cmp, Sprites::SpriteMetaType sprite_type, float zorder )
{
  auto plant_entt = reg.create();
  reg.emplace_or_replace<Cmp::Position>( plant_entt, pos_cmp.position, pos_cmp.size );
  reg.emplace_or_replace<Cmp::PlantObstacle>( plant_entt );
  reg.emplace_or_replace<Cmp::CarryItem>( plant_entt, sprite_type );
  reg.emplace_or_replace<Cmp::ReservedPosition>( plant_entt );
  reg.emplace_or_replace<Cmp::ZOrderValue>( plant_entt, pos_cmp.position.y + zorder );
  reg.emplace_or_replace<Cmp::NpcNoPathFinding>( plant_entt );
  reg.emplace_or_replace<Cmp::AbsoluteAlpha>( plant_entt, 255 );
  reg.emplace_or_replace<Cmp::SpriteAnimation>( plant_entt, 0, 0, true, sprite_type, 0 );
  reg.emplace_or_replace<Cmp::Armable>( plant_entt );
  return plant_entt;
}

void gen_random_plants( entt::registry &reg, Sprites::SpriteFactory &sprite_factory, sf::Vector2u map_grid_size )
{
  auto num_plants = map_grid_size.x * map_grid_size.y / 200;

  for ( std::size_t i = 0; i < num_plants; ++i )
  {
    auto [random_entity, random_pos] = Utils::Rnd::get_random_position(
        reg, {}, Utils::Rnd::ExcludePack<Cmp::PlayerCharacter, Cmp::ReservedPosition, Cmp::Obstacle>{}, 0 );

    // select a random number within the range of possible flora CarryItems
    auto [rand_plant_type, rnd_plant_idx] = sprite_factory.get_random_type_and_texture_index(
        { "CARRYITEM.plant1", "CARRYITEM.plant2", "CARRYITEM.plant3", "CARRYITEM.plant4", "CARRYITEM.plant5", "CARRYITEM.plant6", "CARRYITEM.plant7",
          "CARRYITEM.plant8", "CARRYITEM.plant9", "CARRYITEM.plant10", "CARRYITEM.plant11", "CARRYITEM.plant12" } );

    Factory::createPlantObstacle( reg, random_pos, rand_plant_type, 0.f );
    SPDLOG_INFO( "Created plant at {},{}", random_pos.position.x, random_pos.position.y );
  }
}

} // namespace ProceduralMaze::Factory
