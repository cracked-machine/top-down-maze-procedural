#include <Components/Inventory/CarryItem.hpp>
#include <Components/PlayerWealth.hpp>
#include <Components/RectBounds.hpp>
#include <Systems/LootSystem.hpp>

#include <Audio/SoundBank.hpp>
#include <Components/Armable.hpp>
#include <Components/InventoryWearLevel.hpp>
#include <Components/Persistent/BombBonus.hpp>
#include <Components/Persistent/HealthBonus.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerCadaverCount.hpp>
#include <Components/PlayerHealth.hpp>
#include <Components/PlayerKeysCount.hpp>
#include <Components/Position.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/ZOrderValue.hpp>
#include <Events/CryptRoomEvent.hpp>
#include <Events/UnlockDoorEvent.hpp>
#include <Factory/LootFactory.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Optimizations.hpp>

namespace ProceduralMaze::Sys
{

LootSystem::LootSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  SPDLOG_DEBUG( "LootSystem initialized" );
}

void LootSystem::check_loot_collision()
{
  // Store both loot effects and the player velocities
  struct LootEffect
  {
    entt::entity loot_entity;
    Sprites::SpriteMetaType type;
    entt::entity player_entity;
  };

  std::vector<LootEffect> loot_effects;

  // First pass: detect collisions and gather effects to apply
  // clang-format off
  auto player_collision_view = getReg().view<Cmp::PlayableCharacter, Cmp::Position>();
  auto loot_collision_view = getReg().view<Cmp::Loot, Cmp::Position, Cmp::SpriteAnimation>();
  // clang-format on

  for ( auto [pc_entt, pc_cmp, pc_pos_cmp] : player_collision_view.each() )
  {
    // reduce hitbox to prevent premature pickup
    auto player_hitbox = Cmp::RectBounds( pc_pos_cmp.position, pc_pos_cmp.size, 0.5f );
    for ( auto [loot_entt, loot_cmp, loot_pos_cmp, loot_sprite_anim] : loot_collision_view.each() )
    {
      if ( not Utils::is_visible_in_view( RenderSystem::getGameView(), loot_pos_cmp ) ) continue;

      if ( pc_pos_cmp.findIntersection( loot_pos_cmp ) )
      {
        // Store effect to apply after collision detection
        loot_effects.push_back( { loot_entt, loot_sprite_anim.m_sprite_type, pc_entt } );
      }
    }
  }

  // Second pass: apply effects and remove loots
  for ( const auto &effect : loot_effects )
  {
    if ( !getReg().valid( effect.player_entity ) ) continue;

    auto &pc_cmp = getReg().get<Cmp::PlayableCharacter>( effect.player_entity );
    auto &pc_health_cmp = getReg().get<Cmp::PlayerHealth>( effect.player_entity );

    // Apply the effect
    if ( effect.type == "EXTRA_HEALTH" )
    {
      auto &health_bonus = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::HealthBonus>( getReg() );
      pc_health_cmp.health = std::min( pc_health_cmp.health + health_bonus.get_value(), 100 );
      m_sound_bank.get_effect( "get_loot" ).play();
      Factory::destroyLootDrop( getReg(), effect.loot_entity );
    }
    else if ( effect.type == "WEAPON_BOOST" )
    {
      // update the wear level of the player inventory, if any
      auto inventory_view = getReg().view<Cmp::PlayerInventorySlot>();
      for ( auto [weapons_entity, inventory_slot] : inventory_view.each() )
      {
        if ( inventory_slot.type.contains( "axe" ) or inventory_slot.type.contains( "pickaxe" ) or inventory_slot.type.contains( "shovel" ) )
        {
          auto wear_level_cmp = getReg().try_get<Cmp::InventoryWearLevel>( weapons_entity );
          if ( wear_level_cmp )
          {
            // increase weapon level by 50, up to max level 100
            wear_level_cmp->m_level = std::clamp( wear_level_cmp->m_level + 50.f, 0.f, 100.f );
            m_sound_bank.get_effect( "get_loot" ).play();
            Factory::destroyLootDrop( getReg(), effect.loot_entity );
          }
        }
      }
    }
    else if ( effect.type == "CHAIN_BOMBS" )
    {
      pc_cmp.blast_radius = std::clamp( pc_cmp.blast_radius + 1, 0, 3 );
      m_sound_bank.get_effect( "get_loot" ).play();
      Factory::destroyLootDrop( getReg(), effect.loot_entity );
    }
    else if ( effect.type == "CADAVER_DROP" )
    {
      auto &pc_cadaver_count = getReg().get<Cmp::PlayerCadaverCount>( effect.player_entity );
      pc_cadaver_count.increment_count( 1 );
      m_sound_bank.get_effect( "get_loot" ).play();
      Factory::destroyLootDrop( getReg(), effect.loot_entity );
      m_sound_bank.get_effect( "secret" ).play();
      get_systems_event_queue().trigger( Events::CryptRoomEvent( Events::CryptRoomEvent::Type::EXIT_ALL_PASSAGES ) );
    }
    else if ( effect.type == "LOOT.goldcoin" )
    {
      auto &wealth_cmp = getReg().get<Cmp::PlayerWealth>( effect.player_entity );
      wealth_cmp.wealth += 1;
      m_sound_bank.get_effect( "get_loot" ).play();
      Factory::destroyLootDrop( getReg(), effect.loot_entity );
    }
    else
    {
      SPDLOG_WARN( "Unknown loot type encountered during pickup: {}", effect.type );
      continue;
    }
  }
}

} // namespace ProceduralMaze::Sys