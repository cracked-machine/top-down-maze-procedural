#include <Components/Destructable.hpp>
#include <Components/Persistent/BombBonus.hpp>
#include <Components/Persistent/HealthBonus.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerCandlesCount.hpp>
#include <Components/PlayerHealth.hpp>
#include <Components/PlayerKeysCount.hpp>
#include <Components/PlayerRelicCount.hpp>
#include <Components/WeaponLevel.hpp>
#include <Events/UnlockDoorEvent.hpp>
#include <Systems/LootSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>

namespace ProceduralMaze::Sys
{

LootSystem::LootSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                        Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  // The entt::dispatcher is independent of the registry, so it is safe to bind event handlers in the constructor
  std::ignore = getEventDispatcher().sink<Events::LootContainerDestroyedEvent>().connect<&LootSystem::on_loot_container_destroyed>(
      this );
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
  auto player_collision_view = getReg()
                                   .view<Cmp::PlayableCharacter, Cmp::Position, Cmp::WeaponLevel, Cmp::PlayerKeysCount,
                                         Cmp::PlayerCandlesCount>();
  auto loot_collision_view = getReg().view<Cmp::Loot, Cmp::Position>();

  for ( auto [pc_entt, pc_cmp, pc_pos_cmp, pc_weapon_level, pc_keys_count, pc_candles_count] : player_collision_view.each() )
  {
    for ( auto [loot_entt, loot_cmp, loot_pos_cmp] : loot_collision_view.each() )
    {
      if ( not is_visible_in_view( RenderSystem::getGameView(), loot_pos_cmp ) ) continue;

      if ( pc_pos_cmp.findIntersection( loot_pos_cmp ) )
      {
        // Store effect to apply after collision detection
        loot_effects.push_back( { loot_entt, loot_cmp.m_type, pc_entt } );
      }
    }
  }

  // Second pass: apply effects and remove loots
  for ( const auto &effect : loot_effects )
  {
    if ( !getReg().valid( effect.player_entity ) ) continue;

    auto &pc_cmp = getReg().get<Cmp::PlayableCharacter>( effect.player_entity );
    auto &pc_health_cmp = getReg().get<Cmp::PlayerHealth>( effect.player_entity );
    auto &weapon_level_cmp = getReg().get<Cmp::WeaponLevel>( effect.player_entity );

    // Apply the effect
    if ( effect.type == "EXTRA_HEALTH" )
    {
      auto &health_bonus = get_persistent_component<Cmp::Persistent::HealthBonus>();
      pc_health_cmp.health = std::min( pc_health_cmp.health + health_bonus.get_value(), 100 );
      m_sound_bank.get_effect( "get_loot" ).play();
    }
    else if ( effect.type == "EXTRA_BOMBS" )
    {
      auto &bomb_bonus = get_persistent_component<Cmp::Persistent::BombBonus>();
      if ( pc_cmp.bomb_inventory >= 0 )
      {
        pc_cmp.bomb_inventory += bomb_bonus.get_value();
        m_sound_bank.get_effect( "get_loot" ).play();
      }
    }

    else if ( effect.type == "INFINI_BOMBS" ) { pc_cmp.bomb_inventory = -1; }
    else if ( effect.type == "CHAIN_BOMBS" ) { pc_cmp.blast_radius = std::clamp( pc_cmp.blast_radius + 1, 0, 3 ); }
    else if ( effect.type == "WEAPON_BOOST" )
    {
      // increase weapon level by 50, up to max level 100
      weapon_level_cmp.m_level = std::clamp( weapon_level_cmp.m_level + 50.f, 0.f, 100.f );
      m_sound_bank.get_effect( "get_loot" ).play();
    }
    else if ( effect.type == "CANDLE_DROP" )
    {
      auto &pc_candles_count = getReg().get<Cmp::PlayerCandlesCount>( effect.player_entity );
      pc_candles_count.increment_count( 1 );
      m_sound_bank.get_effect( "get_loot" ).play();
    }
    else if ( effect.type == "KEY_DROP" )
    {
      auto &pc_keys_count = getReg().get<Cmp::PlayerKeysCount>( effect.player_entity );
      pc_keys_count.increment_count( 1 );
      m_sound_bank.get_effect( "get_key" ).play();
      // unlock the door (internally checks if we activated all of the shrines)
      getEventDispatcher().trigger( Events::UnlockDoorEvent() );
    }
    else if ( effect.type == "RELIC_DROP" )
    {
      auto &pc_relic_count = getReg().get<Cmp::PlayerRelicCount>( effect.player_entity );
      pc_relic_count.increment_count( 1 );
      m_sound_bank.get_effect( "get_loot" ).play();
    }
    else
    {
      SPDLOG_WARN( "Unknown loot type encountered during pickup: {}", effect.type );
      continue;
    }

    // Remove the loot component
    getReg().remove<Cmp::Loot>( effect.loot_entity );
    getReg().emplace_or_replace<Cmp::Destructable>( effect.loot_entity );
  }
}

void LootSystem::detonate_loot_container( const Events::LootContainerDestroyedEvent &event )
{
  // the loot container is now destroyed by the bomb, replace with a random loot component
  auto [obstacle_type, random_obstacle_texture_index] = m_sprite_factory.get_random_type_and_texture_index(
      std::vector<std::string>{ "EXTRA_HEALTH", "EXTRA_BOMBS", "INFINI_BOMBS", "CHAIN_BOMBS", "WEAPON_BOOST" } );
  getReg().remove<Cmp::LootContainer>( event.m_entity );
  getReg().remove<Cmp::ReservedPosition>( event.m_entity );
  getReg().emplace_or_replace<Cmp::Loot>( event.m_entity, obstacle_type, random_obstacle_texture_index );
  auto &break_pot_player = m_sound_bank.get_effect( "break_pot" );
  if ( break_pot_player.getStatus() == sf::Sound::Status::Stopped ) { break_pot_player.play(); }
}

} // namespace ProceduralMaze::Sys