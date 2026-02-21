#include <Components/Armable.hpp>
#include <Components/DeathPosition.hpp>
#include <Components/Direction.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Npc/NpcContainer.hpp>
#include <Components/Npc/NpcScanBounds.hpp>
#include <Components/Npc/NpcShockwave.hpp>
#include <Components/Npc/NpcShockwaveTimer.hpp>
#include <Components/Persistent/NpcScanScale.hpp>
#include <Components/Persistent/NpcShockwaveFreq.hpp>
#include <Components/Persistent/NpcShockwaveResolution.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/ReservedPosition.hpp>
#include <Factory/Factory.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/NpcFactory.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Utils/Random.hpp>
#include <entt/entity/entity.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Factory
{

void createNpcContainer( entt::registry &registry, entt::entity entt, Cmp::Position pos_cmp, Sprites::SpriteMetaType sprite_type,
                         std::size_t sprite_tile_idx, float zorder )
{
  registry.emplace_or_replace<Cmp::ReservedPosition>( entt );
  registry.emplace_or_replace<Cmp::Armable>( entt );
  registry.emplace_or_replace<Cmp::NpcContainer>( entt );
  registry.emplace_or_replace<Cmp::SpriteAnimation>( entt, 0, 0, true, sprite_type, sprite_tile_idx );
  registry.emplace_or_replace<Cmp::ZOrderValue>( entt, pos_cmp.position.y - zorder );
}

void destroyNpcContainer( entt::registry &registry, entt::entity npc_container_entity )
{
  registry.remove<Cmp::NpcContainer>( npc_container_entity );
  registry.remove<Cmp::SpriteAnimation>( npc_container_entity );
  registry.remove<Cmp::ZOrderValue>( npc_container_entity );
}

void createShockwave( entt::registry &registry, entt::entity npc_entt )
{
  // get the shockwave timer for the NPC
  auto shockwave_timer = registry.try_get<Cmp::NpcShockwaveTimer>( npc_entt );
  if ( not shockwave_timer )
  {
    SPDLOG_WARN( "Unable to get Cmp::NpcShockwaveTimer from NPC entity" );
    return;
  }

  // check cooldown on this NPC shockwave timer
  sf::Time sw_emit_freq{ sf::milliseconds( Sys::PersistSystem::get<Cmp::Persist::NpcShockwaveFreq>( registry ).get_value() ) };
  if ( shockwave_timer->getElapsedTime() > sw_emit_freq )
  {
    // create a new entity for adding the shockwave component to the NPC position
    auto npc_pos = registry.try_get<Cmp::Position>( npc_entt );
    if ( not npc_pos )
    {
      SPDLOG_WARN( "Unable to get position from NPC entity" );
      return;
    }
    auto npc_sw_entt = registry.create();
    int circle_resolution = Sys::PersistSystem::get<Cmp::Persist::NpcShockwaveResolution>( registry ).get_value();
    registry.emplace_or_replace<Cmp::NpcShockwave>( npc_sw_entt, npc_pos->getCenter(), circle_resolution );

    shockwave_timer->restart(); // make sure we restart the timer
  }
}

void createNPC( entt::registry &registry, entt::entity position_entity, const Sprites::SpriteMetaType &type )
{

  auto pos_cmp = registry.try_get<Cmp::Position>( position_entity );
  if ( not pos_cmp )
  {
    SPDLOG_ERROR( "Cannot add NPC entity {} without a Position component", static_cast<int>( position_entity ) );
    return;
  }

  // create a new entity for the NPC using the existing position
  auto new_pos_entity = registry.create();
  registry.emplace<Cmp::Position>( new_pos_entity, pos_cmp->position, Constants::kGridSizePxF );
  registry.emplace<Cmp::Armable>( new_pos_entity );
  registry.emplace_or_replace<Cmp::Direction>( new_pos_entity, sf::Vector2f{ 0, 0 } );
  auto &npc_scan_scale = Sys::PersistSystem::get<Cmp::Persist::NpcScanScale>( registry );
  registry.emplace_or_replace<Cmp::NPCScanBounds>( new_pos_entity, pos_cmp->position, Constants::kGridSizePxF, npc_scan_scale.get_value() );
  if ( type == "NPCGHOST" )
  {
    registry.emplace_or_replace<Cmp::NPC>( new_pos_entity );
    registry.emplace_or_replace<Cmp::SpriteAnimation>( new_pos_entity, 0, 0, true, "NPCGHOST.walk.east" );
    registry.emplace_or_replace<Cmp::ZOrderValue>( new_pos_entity, pos_cmp->position.y );
    SPDLOG_INFO( "Spawned NPC entity {} of type {} at position ({}, {})", static_cast<int>( new_pos_entity ), type, pos_cmp->position.x,
                 pos_cmp->position.y );
  }
  else if ( type == "NPCSKELE" )
  {
    registry.emplace_or_replace<Cmp::NPC>( new_pos_entity );
    registry.emplace_or_replace<Cmp::SpriteAnimation>( new_pos_entity, 0, 0, true, "NPCSKELE.walk.east" );
    registry.emplace_or_replace<Cmp::ZOrderValue>( new_pos_entity, pos_cmp->position.y );

    // Remove the npc container component from the original entity
    registry.remove<Cmp::NpcContainer>( position_entity );
    registry.remove<Cmp::ZOrderValue>( position_entity );
    SPDLOG_INFO( "Spawned NPC entity {} of type {} at position ({}, {})", static_cast<int>( new_pos_entity ), type, pos_cmp->position.x,
                 pos_cmp->position.y );
  }
  else if ( type == "NPCPRIEST" )
  {
    registry.emplace_or_replace<Cmp::NPC>( new_pos_entity );
    registry.emplace_or_replace<Cmp::SpriteAnimation>( new_pos_entity, 0, 0, false, "NPCPRIEST" );
    registry.emplace_or_replace<Cmp::ZOrderValue>( new_pos_entity, pos_cmp->position.y );

    // Remove the npc container component from the original entity
    registry.remove<Cmp::NpcContainer>( position_entity );
    registry.remove<Cmp::ZOrderValue>( position_entity );

    registry.emplace_or_replace<Cmp::NpcShockwaveTimer>( new_pos_entity );
    Factory::createShockwave( registry, position_entity );
    SPDLOG_INFO( "Spawned NPC entity {} of type {} at position ({}, {})", static_cast<int>( new_pos_entity ), type, pos_cmp->position.x,
                 pos_cmp->position.y );
  }
  else if ( type == "NPCWITCH" )
  {
    registry.emplace_or_replace<Cmp::NPC>( new_pos_entity );
    registry.emplace_or_replace<Cmp::SpriteAnimation>( new_pos_entity, 0, 0, false, "NPCWITCH" );
    registry.emplace_or_replace<Cmp::ZOrderValue>( new_pos_entity, pos_cmp->position.y );

    // Remove the npc container component from the original entity
    registry.remove<Cmp::NpcContainer>( position_entity );
    registry.remove<Cmp::ZOrderValue>( position_entity );
    SPDLOG_INFO( "Spawned NPC entity {} of type {} at position ({}, {})", static_cast<int>( new_pos_entity ), type, pos_cmp->position.x,
                 pos_cmp->position.y );
  }
}

entt::entity destroyNPC( entt::registry &registry, entt::entity npc_entity )
{

  // check for position component
  entt::entity loot_entity = entt::null;
  auto npc_pos_cmp = registry.try_get<Cmp::Position>( npc_entity );
  if ( not npc_pos_cmp ) { SPDLOG_WARN( "Cannot process loot drop for NPC entity {} without a Position component", static_cast<int>( npc_entity ) ); }

  // kill npc once we are done
  registry.remove<Cmp::NPC>( npc_entity );
  registry.remove<Cmp::Position>( npc_entity );
  registry.remove<Cmp::NPCScanBounds>( npc_entity );
  registry.remove<Cmp::Direction>( npc_entity );
  registry.remove<Cmp::SpriteAnimation>( npc_entity );
  registry.remove<Cmp::ZOrderValue>( npc_entity );

  return loot_entity;
}

void createNpcExplosion( entt::registry &registry, Cmp::Position npc_pos_cmp )
{
  auto npc_death_entity = registry.create();
  registry.emplace<Cmp::Position>( npc_death_entity, npc_pos_cmp.position, npc_pos_cmp.size );
  registry.emplace_or_replace<Cmp::DeathPosition>( npc_death_entity, npc_pos_cmp.position, npc_pos_cmp.size );
  registry.emplace_or_replace<Cmp::SpriteAnimation>( npc_death_entity, 0, 0, true, "EXPLOSION", 0 );
  registry.emplace_or_replace<Cmp::ZOrderValue>( npc_death_entity, npc_pos_cmp.position.y );
}

void gen_npc_containers( entt::registry &reg, Sprites::SpriteFactory &sprite_factory, sf::Vector2u map_grid_size )
{
  auto num_npc_containers = map_grid_size.x * map_grid_size.y / 120; // one NPC container per N grid squares

  for ( std::size_t i = 0; i < num_npc_containers; ++i )
  {
    auto [random_entity, random_origin_position] = Utils::Rnd::get_random_position(
        reg, {}, Utils::Rnd::ExcludePack<Cmp::PlayerCharacter, Cmp::ReservedPosition, Cmp::Obstacle>{}, 0 );

    // pick a random loot container type and texture index
    // clang-format off
    auto [npc_type, rand_npc_tex_idx] =
      sprite_factory.get_random_type_and_texture_index( {
        "BONES"
      } );
    // clang-format on

    Factory::createNpcContainer( reg, random_entity, random_origin_position, npc_type, rand_npc_tex_idx, 0.f );
  }
}

} // namespace ProceduralMaze::Factory