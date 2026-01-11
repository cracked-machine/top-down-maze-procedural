#include <Components/Armed.hpp>
#include <Components/DestroyedObstacle.hpp>
#include <Components/NoPathFinding.hpp>
#include <Components/Persistent/ArmedOffDelay.hpp>
#include <Components/Persistent/ArmedOnDelay.hpp>
#include <Components/Persistent/FuseDelay.hpp>
#include <Components/Position.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/BombFactory.hpp>
#include <Systems/PersistSystem.hpp>

#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Factory
{

void createArmed( entt::registry &registry, entt::entity entity, Cmp::Armed::EpiCenter epi_center, int sequence, int zorder )
{

  // get persistent settings
  // clang-format off
  sf::Color color = sf::Color( 255, 10 + ( sequence * 10 ) % 155, 255, 64 );
  auto &fuse_delay = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::FuseDelay>( registry );
  auto &armed_on_delay = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::ArmedOnDelay>( registry );
  auto &armed_off_delay = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::ArmedOffDelay>( registry );
  auto new_fuse_delay = sf::seconds( fuse_delay.get_value() + ( sequence * armed_on_delay.get_value() ) );
  auto new_warning_delay = sf::seconds( armed_off_delay.get_value() + ( sequence * armed_off_delay.get_value() ) );
  // clang-format on

  // create the new armed entity
  auto new_armed_entity = registry.create();
  // clang-format off
    registry.emplace_or_replace<Cmp::Armed>(
      new_armed_entity,
      new_fuse_delay,
      new_warning_delay,
      (epi_center == Cmp::Armed::EpiCenter::YES) ? true : false,
      color,
      sequence,
      epi_center
    );
  // clang-format on

  registry.emplace_or_replace<Cmp::Position>( new_armed_entity, registry.get<Cmp::Position>( entity ).position,
                                              registry.get<Cmp::Position>( entity ).size );

  if ( epi_center == Cmp::Armed::EpiCenter::YES )
  {
    registry.emplace_or_replace<Cmp::SpriteAnimation>( new_armed_entity, 0, 0, true, "CARRYITEM.bomb", 0 );
    registry.emplace_or_replace<Cmp::ZOrderValue>( new_armed_entity, zorder );
    registry.emplace_or_replace<Cmp::NoPathFinding>( new_armed_entity );
  }

  registry.emplace_or_replace<Cmp::NoPathFinding>( new_armed_entity );
}

void destroyArmed( entt::registry &reg, entt::entity armed_entity )
{
  reg.remove<Cmp::Armed>( armed_entity );
  reg.remove<Cmp::SpriteAnimation>( armed_entity );
  reg.remove<Cmp::ZOrderValue>( armed_entity );
  if ( reg.all_of<Cmp::NoPathFinding>( armed_entity ) ) { reg.remove<Cmp::NoPathFinding>( armed_entity ); }
}

void createDetonated( entt::registry &reg, entt::entity armed_entity, Cmp::Position &armed_pos_cmp )
{
  reg.emplace<Cmp::SpriteAnimation>( armed_entity, 0, 0, true, "DETONATED", 0 );
  reg.emplace<Cmp::ZOrderValue>( armed_entity, armed_pos_cmp.position.y - 256.f );
  reg.emplace_or_replace<Cmp::DestroyedObstacle>( armed_entity );
}

} // namespace ProceduralMaze::Factory
