#include <Components/AbsoluteAlpha.hpp>
#include <Components/AbsoluteRotation.hpp>
#include <Components/Direction.hpp>
#include <Components/Neighbours.hpp>
#include <Components/PCDetectionBounds.hpp>
#include <Components/Persistent/BlastRadius.hpp>
#include <Components/Persistent/BombInventory.hpp>
#include <Components/Persistent/PlayerDetectionScale.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerCandlesCount.hpp>
#include <Components/PlayerHealth.hpp>
#include <Components/PlayerKeysCount.hpp>
#include <Components/PlayerMortality.hpp>
#include <Components/PlayerRelicCount.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpawnArea.hpp>
#include <Components/WeaponLevel.hpp>
#include <Factory/PlayerFactory.hpp>
#include <SFML/System/Vector2.hpp>
#include <Systems/PersistentSystem.hpp>
#include <Utils/Utils.hpp>
#include <entt/entity/fwd.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Factory
{

void CreatePlayer( entt::registry &registry )
{
  SPDLOG_INFO( "Creating player entity" );
  auto entity = registry.create();

  // start position must be pixel coordinates within the screen resolution (kDisplaySize),
  // but also grid aligned (kMapGridSize) to avoid collision detection errors.
  // So we must recalc start position to the nearest grid position here
  auto start_pos = Sys::PersistentSystem::get_persistent_component<
      Cmp::Persistent::PlayerStartPosition>( registry );
  start_pos = Utils::snap_to_grid( start_pos );
  registry.emplace<Cmp::Position>( entity, start_pos, Constants::kGridSquareSizePixelsF );

  auto &bomb_inventory = Sys::PersistentSystem::get_persistent_component<
      Cmp::Persistent::BombInventory>( registry );
  auto
      &blast_radius = Sys::PersistentSystem::get_persistent_component<Cmp::Persistent::BlastRadius>(
          registry );
  registry.emplace<Cmp::PlayableCharacter>( entity, bomb_inventory.get_value(),
                                            blast_radius.get_value() );

  registry.emplace<Cmp::Direction>( entity, sf::Vector2f{ 0, 0 } );

  auto &pc_detection_scale = Sys::PersistentSystem::get_persistent_component<
      Cmp::Persistent::PlayerDetectionScale>( registry );

  registry.emplace<Cmp::PCDetectionBounds>( entity, start_pos, Constants::kGridSquareSizePixelsF,
                                            pc_detection_scale.get_value() );

  registry.emplace<Cmp::SpriteAnimation>( entity, 0, 0, true, "PLAYER.walk.south" );
  registry.emplace<Cmp::PlayerCandlesCount>( entity, 0 );
  registry.emplace<Cmp::PlayerKeysCount>( entity, 0 );
  registry.emplace<Cmp::PlayerRelicCount>( entity, 0 );
  registry.emplace<Cmp::PlayerHealth>( entity, 100 );
  registry.emplace<Cmp::PlayerMortality>( entity, Cmp::PlayerMortality::State::ALIVE );
  registry.emplace<Cmp::WeaponLevel>( entity, 100.f );
  registry.emplace<Cmp::ZOrderValue>( entity, start_pos.y ); // z-order based on y-position
  registry.emplace<Cmp::AbsoluteAlpha>( entity, 255 );       // fully opaque
  registry.emplace<Cmp::AbsoluteRotation>( entity, 0 );
}

entt::entity createWorldPosition( entt::registry &registry, const sf::Vector2f &pos )
{
  auto entity = registry.create();
  registry.emplace_or_replace<Cmp::Position>( entity, pos, Constants::kGridSquareSizePixelsF );
  registry.emplace_or_replace<Cmp::Neighbours>( entity );
  return entity;
}

void addSpawnArea( entt::registry &registry, entt::entity entity, float zorder )
{
  // We need to reserve these positions for the player start area
  registry.emplace_or_replace<Cmp::ReservedPosition>( entity );
  registry.emplace_or_replace<Cmp::SpawnArea>( entity, false );
  registry.emplace_or_replace<Cmp::NoPathFinding>( entity );
  registry.emplace_or_replace<Cmp::SpriteAnimation>( entity, 0, 0, true, "PLAYERSPAWN", 0 );
  registry.emplace_or_replace<Cmp::ZOrderValue>( entity, zorder );
}

} // namespace ProceduralMaze::Factory
