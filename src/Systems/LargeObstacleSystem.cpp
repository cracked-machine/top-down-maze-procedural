#include <Components/AltarMultiBlock.hpp>
#include <Components/GraveMultiBlock.hpp>
#include <Components/GraveSegment.hpp>
#include <Components/LootContainer.hpp>
#include <Components/NPC.hpp>
#include <Components/Persistent/ShrineCostPerSprite.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerCandlesCount.hpp>
#include <Components/PlayerKeysCount.hpp>
#include <Components/PlayerRelicCount.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ShrineSegment.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Events/LootContainerDestroyedEvent.hpp>
#include <Events/NpcCreationEvent.hpp>
#include <Events/NpcDeathEvent.hpp>
#include <Systems/LargeObstacleSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>

namespace ProceduralMaze::Sys
{

LargeObstacleSystem::LargeObstacleSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                                          Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  // The entt::dispatcher is independent of the registry, so it is safe to bind event handlers in the constructor
  std::ignore = get_systems_event_queue().sink<Events::PlayerActionEvent>().connect<&LargeObstacleSystem::on_player_action>( this );
}

void LargeObstacleSystem::check_player_lo_collision( Events::PlayerActionEvent::GameActions action )
{
  if ( action != Events::PlayerActionEvent::GameActions::ACTIVATE ) return;

  auto player_view = getReg().view<Cmp::PlayableCharacter, Cmp::Position, Cmp::PlayerCandlesCount, Cmp::PlayerKeysCount>();
  auto altar_view = getReg().view<Cmp::AltarMultiBlock>();
  auto grave_view = getReg().view<Cmp::GraveMultiBlock>();

  for ( auto [pc_entity, pc_cmp, pc_pos_cmp, pc_candles_cmp, pc_keys_cmp] : player_view.each() )
  {
    auto player_hitbox = Cmp::RectBounds( pc_pos_cmp.position, kGridSquareSizePixelsF, 1.5f );

    for ( auto [altar_entity, altar_cmp] : altar_view.each() )
    {
      if ( player_hitbox.findIntersection( altar_cmp ) )
      {
        SPDLOG_DEBUG( "Player collided with Altar at ({}, {})", altar_cmp.position.x, altar_cmp.position.y );
        check_player_altar_activation( altar_entity, altar_cmp, pc_candles_cmp );
      }
    }

    for ( auto [grave_entity, grave_cmp] : grave_view.each() )
    {
      if ( player_hitbox.findIntersection( grave_cmp ) )
      {
        SPDLOG_DEBUG( "Player collided with Grave at ({}, {})", grave_cmp.position.x, grave_cmp.position.y );
        check_player_grave_activation( grave_entity, grave_cmp, pc_cmp );
      }
    }
  }
}

void LargeObstacleSystem::check_player_altar_activation( entt::entity altar_entity, Cmp::AltarMultiBlock &altar_cmp,
                                                         Cmp::PlayerCandlesCount &pc_candles_cmp )
{

  SPDLOG_DEBUG( "Checking altar activation: {}/{}", altar_cmp.get_activation_count(), altar_cmp.get_activation_threshold() );
  if ( altar_cmp.get_activation_count() < altar_cmp.get_activation_threshold() )
  {
    if ( pc_candles_cmp.get_count() < 1 ) return;
    switch ( altar_cmp.get_activation_count() )
    {
      case 0:
        // clang-format off
          getReg().patch<Cmp::AltarMultiBlock>( altar_entity, [&]( Cmp::AltarMultiBlock &altar_cmp ) { 
            altar_cmp.set_activation_count( 1 );
          } );
          getReg().patch<Cmp::SpriteAnimation>( altar_entity, [&]( Cmp::SpriteAnimation &anim_cmp ) {
            anim_cmp.m_sprite_type = "SHRINE.one";
          } );

        // clang-format on
        pc_candles_cmp.decrement_count( 1 );
        SPDLOG_INFO( "Shrine activated to state ONE." );
        break;
      case 1:
        // clang-format off
          getReg().patch<Cmp::AltarMultiBlock>( altar_entity, [&]( Cmp::AltarMultiBlock &altar_cmp ) { 
            altar_cmp.set_activation_count( 2 );
          } );
          getReg().patch<Cmp::SpriteAnimation>( altar_entity, [&]( Cmp::SpriteAnimation &anim_cmp ) {
            anim_cmp.m_sprite_type = "SHRINE.two";
          } );

        // clang-format on
        pc_candles_cmp.decrement_count( 1 );
        SPDLOG_INFO( "Shrine activated to state TWO." );
        break;

      case 2:
        // clang-format off
          getReg().patch<Cmp::AltarMultiBlock>( altar_entity, [&]( Cmp::AltarMultiBlock &altar_cmp ) { 
            altar_cmp.set_activation_count( 3 ); 
          } );
          getReg().patch<Cmp::SpriteAnimation>( altar_entity, [&]( Cmp::SpriteAnimation &anim_cmp ) {
            anim_cmp.m_sprite_type = "SHRINE.three";
          } );

        // clang-format on
        pc_candles_cmp.decrement_count( 1 );
        SPDLOG_INFO( "Shrine activated to state THREE." );
        break;
      case 3:
        // clang-format off
          getReg().patch<Cmp::AltarMultiBlock>( altar_entity, [&]( Cmp::AltarMultiBlock &altar_cmp ) { 
            altar_cmp.set_activation_count( 4 ); 
          } );
          getReg().patch<Cmp::SpriteAnimation>( altar_entity, [&]( Cmp::SpriteAnimation &anim_cmp ) {
            anim_cmp.m_sprite_type = "SHRINE.four";
          } );

        // clang-format on
        pc_candles_cmp.decrement_count( 1 );
        SPDLOG_INFO( "Shrine activated to state FOUR." );
        break;
      default:
        break;
    }
  }

  // allow this condition to be met in the same pass as the candle activation above
  if ( altar_cmp.get_activation_count() >= altar_cmp.get_activation_threshold() )
  {
    if ( not altar_cmp.are_powers_active() )
    {
      SPDLOG_INFO( "Shrine fully activated!" );
      m_sound_bank.get_effect( "shrine_lighting" ).play();
      // drop the key loot
      auto altar_cmp_bounds = Cmp::RectBounds( altar_cmp.position, altar_cmp.size, 1.5f );
      // clang-format off
      auto obst_entity = create_loot_drop( 
        Cmp::Loot{ "KEY_DROP", 0 },
        sf::FloatRect{ altar_cmp_bounds.position(), 
        altar_cmp_bounds.size() }, 
        IncludePack<>{},
        ExcludePack<>{} 
      );
      // clang-format on
      if ( obst_entity != entt::null ) { m_sound_bank.get_effect( "drop_loot" ).play(); }
      altar_cmp.set_powers_active();
      m_shrine_activation_clock.restart();
    }

    if ( altar_cmp.are_powers_active() )
    {
      SPDLOG_DEBUG( "Checking for special power activation." );
      auto anim_sprite_cmp = getReg().try_get<Cmp::SpriteAnimation>( altar_entity );
      if ( anim_sprite_cmp && m_shrine_activation_clock.getElapsedTime() > m_shrine_activation_cooldown )
      {
        activate_altar_special_power();
      }
    }
  }
}

void LargeObstacleSystem::check_player_grave_activation( entt::entity &grave_entity, Cmp::GraveMultiBlock &grave_cmp,
                                                         Cmp::PlayableCharacter &pc_cmp )
{
  if ( grave_cmp.are_powers_active() ) return;
  if ( grave_cmp.get_activation_count() < grave_cmp.get_activation_threshold() )
  {

    SPDLOG_DEBUG( "Activating grave sprite {}/{}.", grave_cmp.get_activated_sprite_count() + 1,
                  grave_cmp.get_activation_threshold() );
    grave_cmp.set_activation_count( grave_cmp.get_activation_threshold() );

    auto anim_cmp = getReg().try_get<Cmp::SpriteAnimation>( grave_entity );
    if ( anim_cmp )
    {
      if ( std::string::size_type n = anim_cmp->m_sprite_type.find( "." ); n != std::string::npos )
      {
        anim_cmp->m_sprite_type = anim_cmp->m_sprite_type.substr( 0, n ) + ".opened";
        SPDLOG_DEBUG( "Grave Cmp::SpriteAnimation changed to opened type: {}", anim_cmp->m_sprite_type );
      }
    }

    grave_cmp.set_powers_active( true );

    // choose a random consequence for activating graves: spawn npc, drop bomb, give candles
    if ( grave_cmp.are_powers_active() )
    {
      auto grave_activation_rng = Cmp::RandomInt( 1, 3 );
      auto consequence = grave_activation_rng.gen();
      switch ( consequence )
      {
        case 1:
          SPDLOG_INFO( "Grave activated NPC trap." );
          get_systems_event_queue().trigger( Events::NpcCreationEvent( grave_entity, "NPCGHOST" ) );
          break;
        case 2:
          SPDLOG_INFO( "Grave activated bomb trap." );
          pc_cmp.bomb_inventory += 1;
          get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::GRAVE_BOMB ) );
          break;
        case 3:

          auto grave_cmp_bounds = Cmp::RectBounds( grave_cmp.position, grave_cmp.size, 1.5f );
          // clang-format off
              auto obst_entity = create_loot_drop( 
                Cmp::Loot{ "CANDLE_DROP", 0 },
                sf::FloatRect{ grave_cmp_bounds.position(), 
                grave_cmp_bounds.size() }, 
                IncludePack<>{},
                ExcludePack<>{} 
              );
          // clang-format on

          if ( obst_entity != entt::null ) { m_sound_bank.get_effect( "drop_loot" ).play(); }
          break;
      }
      // }
    }
  }
}

bool LargeObstacleSystem::activate_altar_special_power()
{

  for ( auto [pc_entity, pc_relic_count_cmp] : getReg().view<Cmp::PlayerRelicCount>().each() )
  {

    if ( pc_relic_count_cmp.get_count() < 1 ) return false;

    SPDLOG_INFO( "Activating altar special power!" );
    // consume a relic
    pc_relic_count_cmp.decrement_count( 1 );

    // Implement the special power activation logic here
    auto special_power_picker = Cmp::RandomInt( 1, 3 );
    auto power_choice = special_power_picker.gen();
    switch ( power_choice )
    {
      case 1:
      {
        SPDLOG_INFO( "Special Power: Kill all nearby NPCs!" );
        auto npc_view = getReg().view<Cmp::NPC, Cmp::Position>();
        for ( auto [npc_entity, npc_cmp, npc_pos_cmp] : npc_view.each() )
        {
          if ( is_visible_in_view( RenderSystem::getGameView(), npc_pos_cmp ) )
          {
            SPDLOG_INFO( "Killed NPC at ({}, {})", npc_pos_cmp.position.x, npc_pos_cmp.position.y );
            get_systems_event_queue().trigger( Events::NpcDeathEvent( npc_entity ) );
          }
        }
        break;
      }
      case 2:
      {
        SPDLOG_INFO( "Special Power: Re-enable all nearby obstacles!" );
        auto obstacle_view = getReg().view<Cmp::Obstacle, Cmp::Position>();
        for ( auto [obst_entity, obst_cmp, obst_pos_cmp] : obstacle_view.each() )
        {
          if ( is_visible_in_view( RenderSystem::getGameView(), obst_pos_cmp ) )
          {
            // only re-enable obstacles the player destroyed
            if ( not obst_cmp.m_enabled && obst_cmp.m_integrity <= 0.0f )
            {
              SPDLOG_INFO( "Re-enabled obstacle at ({}, {})", obst_pos_cmp.position.x, obst_pos_cmp.position.y );
              obst_cmp.m_enabled = true;
              obst_cmp.m_integrity = 1.0f;
            }
          }
        }
        break;
      }
      case 3:
      {
        SPDLOG_INFO( "Special Power: Open all loot containers!" );
        auto loot_container_view = getReg().view<Cmp::LootContainer, Cmp::Position>();
        for ( auto [lc_entity, lc_cmp, lc_pos_cmp] : loot_container_view.each() )
        {
          if ( is_visible_in_view( RenderSystem::getGameView(), lc_pos_cmp ) )
          {
            SPDLOG_INFO( "Opened loot container at ({}, {})", lc_pos_cmp.position.x, lc_pos_cmp.position.y );
            get_systems_event_queue().trigger( Events::LootContainerDestroyedEvent( lc_entity ) );
          }
        }
        break;
      }
    }
  }
  m_shrine_activation_clock.restart();
  return true;
}

} // namespace ProceduralMaze::Sys