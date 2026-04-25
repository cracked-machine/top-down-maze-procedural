#include <Player.hpp>
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#include <Audio/SoundBank.hpp>
#include <Components/Armable.hpp>
#include <Components/Inventory/FlashUICadaver.hpp>
#include <Components/Inventory/FlashUIRadius.hpp>
#include <Components/Inventory/InventoryItem.hpp>
#include <Components/Inventory/InventoryWearLevel.hpp>
#include <Components/Persistent/BombBonus.hpp>
#include <Components/Persistent/HealthBonus.hpp>
#include <Components/Player/PlayerBlastRadius.hpp>
#include <Components/Player/PlayerCadaverCount.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerKeysCount.hpp>
#include <Components/Player/PlayerWealth.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/ZOrderValue.hpp>
#include <Events/CryptRoomEvent.hpp>
#include <Events/UnlockDoorEvent.hpp>
#include <Factory/LootFactory.hpp>
#include <Systems/LootSystem.hpp>
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
  auto player_collision_view = reg().view<Cmp::PlayerCharacter, Cmp::Position>();
  auto loot_collision_view = reg().view<Cmp::Loot, Cmp::Position, Cmp::SpriteAnimation>();
  // clang-format on

  for ( auto [pc_entt, pc_cmp, pc_pos_cmp] : player_collision_view.each() )
  {
    // reduce hitbox to prevent premature pickup
    auto player_hitbox = Cmp::RectBounds::scaled( pc_pos_cmp.position, pc_pos_cmp.size, 0.5f );
    for ( auto [loot_entt, loot_cmp, loot_pos_cmp, loot_sprite_anim] : loot_collision_view.each() )
    {
      if ( not Utils::is_visible_in_view( RenderSystem::get_world_view(), loot_pos_cmp ) ) continue;

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
    if ( !reg().valid( effect.player_entity ) ) continue;

    auto blast_radius = reg().get<Cmp::PlayerBlastRadius>( effect.player_entity );

    // Apply the effect
    if ( effect.type == "EXTRA_HEALTH" )
    {
      auto &health_bonus = Sys::PersistSystem::get<Cmp::Persist::HealthBonus>( reg() );

      // pc_health_cmp.health = std::min( pc_health_cmp.health + health_bonus.get_value(), 100 );
      Utils::Player::get_player_stats( reg() ).apply_modifiers( { Cmp::Stats::Health{ health_bonus.get_value() }, {}, {}, {}, {} } );
      m_sound_bank.get_effect( "get_loot" ).play();
      Factory::destroy_loot_drop( reg(), effect.loot_entity );
    }
    else if ( effect.type == "WEAPON_BOOST" )
    {
      // update the wear level of the player inventory, if any
      auto inventory_view = reg().view<Cmp::PlayerInventorySlot>();
      for ( auto [weapons_entity, inventory_slot] : inventory_view.each() )
      {
        if ( inventory_slot.m_item.sprite_type.contains( "axe" ) or inventory_slot.m_item.sprite_type.contains( "pickaxe" ) or
             inventory_slot.m_item.sprite_type.contains( "shovel" ) )
        {
          auto wear_level_cmp = reg().try_get<Cmp::InventoryWearLevel>( weapons_entity );
          if ( wear_level_cmp )
          {
            // increase weapon level by 50, up to max level 100
            wear_level_cmp->m_level = std::clamp( wear_level_cmp->m_level + 50.f, 0.f, 100.f );
            m_sound_bank.get_effect( "get_loot" ).play();
            Factory::destroy_loot_drop( reg(), effect.loot_entity );
          }
        }
      }
    }
    else if ( effect.type == "CHAIN_BOMBS" )
    {
      blast_radius.value = std::clamp( blast_radius.value + 1, 0, 5 );
      m_sound_bank.get_effect( "get_loot" ).play();
      Factory::destroy_loot_drop( reg(), effect.loot_entity );

      // signal UI to flash
      auto flash_entt = reg().create();
      reg().emplace_or_replace<Cmp::FlashUIRadius>( flash_entt );
    }
    else if ( effect.type == "CADAVER_DROP" )
    {
      auto &pc_cadaver_count = reg().get<Cmp::PlayerCadaverCount>( effect.player_entity );
      pc_cadaver_count.increment_count( 1 );
      m_sound_bank.get_effect( "get_loot" ).play();
      Factory::destroy_loot_drop( reg(), effect.loot_entity );
      m_sound_bank.get_effect( "secret" ).play();

      // signal UI to flash
      auto flash_entt = reg().create();
      reg().emplace_or_replace<Cmp::FlashUICadaver>( flash_entt );

      get_systems_event_queue().trigger( Events::CryptRoomEvent( Events::CryptRoomEvent::Type::EXIT_ALL_PASSAGES ) );
    }
    else if ( effect.type == "LOOT.goldcoin" )
    {
      auto &wealth_cmp = reg().get<Cmp::PlayerWealth>( effect.player_entity );
      wealth_cmp.wealth += 1;
      m_sound_bank.get_effect( "get_loot" ).play();
      Factory::destroy_loot_drop( reg(), effect.loot_entity );
    }
    else
    {
      SPDLOG_WARN( "Unknown loot type encountered during pickup: {}", effect.type );
      continue;
    }
  }
}

} // namespace ProceduralMaze::Sys