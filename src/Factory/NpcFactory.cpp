#include <Components/Armable.hpp>
#include <Components/Direction.hpp>
#include <Components/NPC.hpp>
#include <Components/NPCScanBounds.hpp>
#include <Components/NpcContainer.hpp>
#include <Components/NpcDeathPosition.hpp>
#include <Components/Persistent/NpcScanScale.hpp>
#include <Components/ReservedPosition.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/NpcFactory.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Systems/BaseSystem.hpp>

#include <entt/entity/entity.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Factory
{

void createNpcContainer( entt::registry &registry, entt::entity entt, Cmp::Position pos_cmp,
                         Sprites::SpriteMetaType sprite_type, std::size_t sprite_tile_idx,
                         float zorder )
{
  registry.emplace_or_replace<Cmp::ReservedPosition>( entt );
  registry.emplace_or_replace<Cmp::Armable>( entt );
  registry.emplace_or_replace<Cmp::NpcContainer>( entt );
  registry.emplace_or_replace<Cmp::SpriteAnimation>( entt, 0, 0, true, sprite_type,
                                                     sprite_tile_idx );
  registry.emplace_or_replace<Cmp::ZOrderValue>( entt, pos_cmp.position.y - zorder );
}

void destroyNpcContainer( entt::registry &registry, entt::entity npc_container_entity )
{
  registry.remove<Cmp::NpcContainer>( npc_container_entity );
  registry.remove<Cmp::SpriteAnimation>( npc_container_entity );
  registry.remove<Cmp::ZOrderValue>( npc_container_entity );
}

void createNPC( entt::registry &registry, entt::entity position_entity,
                const Sprites::SpriteMetaType &type )
{

  auto pos_cmp = registry.try_get<Cmp::Position>( position_entity );
  if ( not pos_cmp )
  {
    SPDLOG_ERROR( "Cannot add NPC entity {} without a Position component",
                  static_cast<int>( position_entity ) );
    return;
  }

  // create a new entity for the NPC using the existing position
  auto new_pos_entity = registry.create();
  registry.emplace<Cmp::Position>( new_pos_entity, pos_cmp->position,
                                   Constants::kGridSquareSizePixelsF );
  registry.emplace<Cmp::Armable>( new_pos_entity );
  registry.emplace_or_replace<Cmp::Direction>( new_pos_entity, sf::Vector2f{ 0, 0 } );
  auto &npc_scan_scale = Sys::BaseSystem::get_persistent_component<Cmp::Persistent::NpcScanScale>(
      registry );
  registry.emplace_or_replace<Cmp::NPCScanBounds>( new_pos_entity, pos_cmp->position,
                                                   Constants::kGridSquareSizePixelsF,
                                                   npc_scan_scale.get_value() );
  if ( type == "NPCGHOST" )
  {
    registry.emplace_or_replace<Cmp::NPC>( new_pos_entity );
    registry.emplace_or_replace<Cmp::SpriteAnimation>( new_pos_entity, 0, 0, true,
                                                       "NPCGHOST.walk.east" );
    registry.emplace_or_replace<Cmp::ZOrderValue>( new_pos_entity, pos_cmp->position.y );
  }
  else if ( type == "NPCSKELE" )
  {
    registry.emplace_or_replace<Cmp::NPC>( new_pos_entity );
    registry.emplace_or_replace<Cmp::SpriteAnimation>( new_pos_entity, 0, 0, true,
                                                       "NPCSKELE.walk.east" );
    registry.emplace_or_replace<Cmp::ZOrderValue>( new_pos_entity, pos_cmp->position.y );

    // Remove the npc container component from the original entity
    registry.remove<Cmp::NpcContainer>( position_entity );
    registry.remove<Cmp::ZOrderValue>( position_entity );
  }

  if ( type == "NPCGHOST" )
  {
    SPDLOG_INFO( "Spawned NPC entity {} of type {} at position ({}, {})",
                 static_cast<int>( new_pos_entity ), type, pos_cmp->position.x,
                 pos_cmp->position.y );
  }
  else if ( type == "NPCSKELE" )
  {
    SPDLOG_INFO( "Spawned NPC entity {} of type {} at position ({}, {})",
                 static_cast<int>( new_pos_entity ), type, pos_cmp->position.x,
                 pos_cmp->position.y );
  }
}

entt::entity destroyNPC( entt::registry &registry, entt::entity npc_entity )
{

  // check for position component
  entt::entity loot_entity = entt::null;
  auto npc_pos_cmp = registry.try_get<Cmp::Position>( npc_entity );
  if ( not npc_pos_cmp )
  {
    SPDLOG_WARN( "Cannot process loot drop for NPC entity {} without a Position component",
                 static_cast<int>( npc_entity ) );
  }
  else
  {
    // 1 in 20 chance of dropping a relic
    auto loot_chance_rng = Cmp::RandomInt( 1, 10 );
    if ( loot_chance_rng.gen() == 1 )
    {
      auto npc_pos_cmp_bounds = Cmp::RectBounds( npc_pos_cmp->position,
                                                 Constants::kGridSquareSizePixelsF, 1.5f );
      // clang-format off
      loot_entity = Factory::createLootDrop(registry,
        Cmp::SpriteAnimation( 0,0, true,"RELIC_DROP", 0 ),
        sf::FloatRect{ npc_pos_cmp_bounds.position(), npc_pos_cmp_bounds.size() },
        Sys::BaseSystem::IncludePack<>{},
        Sys::BaseSystem::ExcludePack<>{}
      );
      // clang-format on
    }
  }

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
  registry.emplace_or_replace<Cmp::NpcDeathPosition>( npc_death_entity, npc_pos_cmp.position,
                                                      npc_pos_cmp.size );
  registry.emplace_or_replace<Cmp::SpriteAnimation>( npc_death_entity, 0, 0, true, "EXPLOSION", 0 );
  registry.emplace_or_replace<Cmp::ZOrderValue>( npc_death_entity, npc_pos_cmp.position.y );
}

} // namespace ProceduralMaze::Factory