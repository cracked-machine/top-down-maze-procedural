#include <Components/AbsoluteAlpha.hpp>
#include <Components/AltarMultiBlock.hpp>
#include <Components/AltarSegment.hpp>
#include <Components/Armable.hpp>
#include <Components/DestroyedObstacle.hpp>
#include <Components/LootContainer.hpp>
#include <Components/NPC.hpp>
#include <Components/NoPathFinding.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerCandlesCount.hpp>
#include <Components/PlayerKeysCount.hpp>
#include <Components/PlayerRelicCount.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpawnArea.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Systems/AltarSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Utils.hpp>

namespace ProceduralMaze::Sys
{

AltarSystem::AltarSystem( entt::registry &reg, sf::RenderWindow &window,
                          Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  // The entt::dispatcher is independent of the registry, so it is safe to bind event handlers in
  // the constructor
  std::ignore = get_systems_event_queue()
                    .sink<Events::PlayerActionEvent>()
                    .connect<&AltarSystem::on_player_action>( this );
}

void AltarSystem::check_player_collision( Events::PlayerActionEvent::GameActions action )
{
  if ( action != Events::PlayerActionEvent::GameActions::ACTIVATE ) return;

  auto player_view = getReg()
                         .view<Cmp::PlayableCharacter, Cmp::Position, Cmp::PlayerCandlesCount,
                               Cmp::PlayerKeysCount>();
  auto altar_view = getReg().view<Cmp::AltarMultiBlock>();

  for ( auto [pc_entity, pc_cmp, pc_pos_cmp, pc_candles_cmp, pc_keys_cmp] : player_view.each() )
  {
    auto player_hitbox = Cmp::RectBounds( pc_pos_cmp.position, Constants::kGridSquareSizePixelsF,
                                          1.5f );

    for ( auto [altar_entity, altar_cmp] : altar_view.each() )
    {
      if ( player_hitbox.findIntersection( altar_cmp ) )
      {
        SPDLOG_DEBUG( "Player collided with Altar at ({}, {})", altar_cmp.position.x,
                      altar_cmp.position.y );
        check_player_altar_activation( altar_entity, altar_cmp, pc_candles_cmp );
      }
    }
  }
}

void AltarSystem::check_player_altar_activation( entt::entity altar_entity,
                                                 Cmp::AltarMultiBlock &altar_cmp,
                                                 Cmp::PlayerCandlesCount &pc_candles_cmp )
{

  SPDLOG_DEBUG( "Checking altar activation: {}/{}", altar_cmp.get_activation_count(),
                altar_cmp.get_activation_threshold() );
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
            anim_cmp.m_sprite_type = "ALTAR.one";
          } );

        // clang-format on
        pc_candles_cmp.decrement_count( 1 );
        SPDLOG_DEBUG( "Altar activated to state ONE." );
        break;
      case 1:
        // clang-format off
          getReg().patch<Cmp::AltarMultiBlock>( altar_entity, [&]( Cmp::AltarMultiBlock &altar_cmp ) { 
            altar_cmp.set_activation_count( 2 );
          } );
          getReg().patch<Cmp::SpriteAnimation>( altar_entity, [&]( Cmp::SpriteAnimation &anim_cmp ) {
            anim_cmp.m_sprite_type = "ALTAR.two";
          } );

        // clang-format on
        pc_candles_cmp.decrement_count( 1 );
        SPDLOG_DEBUG( "Altar activated to state TWO." );
        break;

      case 2:
        // clang-format off
          getReg().patch<Cmp::AltarMultiBlock>( altar_entity, [&]( Cmp::AltarMultiBlock &altar_cmp ) { 
            altar_cmp.set_activation_count( 3 ); 
          } );
          getReg().patch<Cmp::SpriteAnimation>( altar_entity, [&]( Cmp::SpriteAnimation &anim_cmp ) {
            anim_cmp.m_sprite_type = "ALTAR.three";
          } );

        // clang-format on
        pc_candles_cmp.decrement_count( 1 );
        SPDLOG_DEBUG( "Altar activated to state THREE." );
        break;
      case 3:
        // clang-format off
          getReg().patch<Cmp::AltarMultiBlock>( altar_entity, [&]( Cmp::AltarMultiBlock &altar_cmp ) { 
            altar_cmp.set_activation_count( 4 ); 
          } );
          getReg().patch<Cmp::SpriteAnimation>( altar_entity, [&]( Cmp::SpriteAnimation &anim_cmp ) {
            anim_cmp.m_sprite_type = "ALTAR.four";
          } );

        // clang-format on
        pc_candles_cmp.decrement_count( 1 );
        SPDLOG_DEBUG( "Altar activated to state FOUR." );
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
      SPDLOG_DEBUG( "Altar fully activated!" );
      m_sound_bank.get_effect( "shrine_lighting" ).play();
      // drop the key loot
      auto altar_cmp_bounds = Cmp::RectBounds( altar_cmp.position, altar_cmp.size, 2.f );
      // clang-format off
      auto obst_entity = Factory::createLootDrop(getReg(),
        Cmp::SpriteAnimation{0,0,true, "KEY_DROP", 0 },
        sf::FloatRect{ altar_cmp_bounds.position(), 
        altar_cmp_bounds.size() }, 
        IncludePack<>{},
        ExcludePack<Cmp::PlayableCharacter, Cmp::AltarSegment, Cmp::SpawnArea>{} 
      );
      // clang-format on
      if ( obst_entity != entt::null ) { m_sound_bank.get_effect( "drop_loot" ).play(); }
      altar_cmp.set_powers_active();
      m_altar_activation_clock.restart();
    }

    if ( altar_cmp.are_powers_active() )
    {
      SPDLOG_DEBUG( "Checking for special power activation." );
      auto anim_sprite_cmp = getReg().try_get<Cmp::SpriteAnimation>( altar_entity );
      if ( anim_sprite_cmp &&
           m_altar_activation_clock.getElapsedTime() > m_altar_activation_cooldown )
      {
        activate_altar_special_power();
      }
    }
  }
}

bool AltarSystem::activate_altar_special_power()
{

  for ( auto [pc_entity, pc_relic_count_cmp] : getReg().view<Cmp::PlayerRelicCount>().each() )
  {

    if ( pc_relic_count_cmp.get_count() < 1 ) return false;

    SPDLOG_DEBUG( "Activating altar special power!" );
    // consume a relic
    pc_relic_count_cmp.decrement_count( 1 );

    SPDLOG_DEBUG( "Special Power: Re-enable all nearby obstacles!" );
    auto obstacle_view = getReg().view<Cmp::DestroyedObstacle, Cmp::Position>(
        entt::exclude<Cmp::PlayableCharacter, Cmp::NPC, Cmp::LootContainer, Cmp::Loot,
                      Cmp::ReservedPosition> );

    for ( auto [destroyed_entity, destroyed_cmp, destroyed_pos_cmp] : obstacle_view.each() )
    {
      if ( Utils::is_visible_in_view( RenderSystem::getGameView(), destroyed_pos_cmp ) )
      {
        auto obst_cmp = getReg().try_get<Cmp::Obstacle>( destroyed_entity );
        if ( not obst_cmp )
        {
          // skip if player or npc is occupying the position
          bool skip_position = false;
          auto npc_view = getReg().view<Cmp::NPC, Cmp::Position>();
          for ( auto [npc_entity, npc_cmp, npc_pos_cmp] : npc_view.each() )
          {
            if ( npc_pos_cmp.findIntersection( destroyed_pos_cmp ) ) skip_position = true;
          }
          auto playable_view = getReg().view<Cmp::PlayableCharacter, Cmp::Position>();
          for ( auto [playable_entity, playable_cmp, playable_pos_cmp] : playable_view.each() )
          {
            if ( playable_pos_cmp.findIntersection( destroyed_pos_cmp ) ) skip_position = true;
          }
          if ( skip_position ) continue;

          // clang-format off
          auto [obst_type, rand_obst_tex_idx] = 
            m_sprite_factory.get_random_type_and_texture_index( { 
              "ROCK"
            } );
          // clang-format on

          Factory::createObstacle( getReg(), destroyed_entity, destroyed_pos_cmp, obst_type,
                                   rand_obst_tex_idx, destroyed_pos_cmp.position.y );
        }
      }
    }
  }

  SPDLOG_DEBUG( "Special Power: Kill all nearby NPCs!" );
  auto npc_view = getReg().view<Cmp::NPC, Cmp::Position>();
  for ( auto [npc_entity, npc_cmp, npc_pos_cmp] : npc_view.each() )
  {
    if ( Utils::is_visible_in_view( RenderSystem::getGameView(), npc_pos_cmp ) )
    {
      SPDLOG_DEBUG( "Killed NPC at ({}, {})", npc_pos_cmp.position.x, npc_pos_cmp.position.y );
      auto loot_entity = Factory::destroyNPC( getReg(), npc_entity );
      if ( loot_entity != entt::null )
      {
        SPDLOG_INFO( "Dropped RELIC_DROP loot at NPC death position." );
        m_sound_bank.get_effect( "drop_relic" ).play();
      }
    }
  }

  SPDLOG_DEBUG( "Special Power: Open all loot containers!" );
  auto loot_container_view = getReg().view<Cmp::LootContainer, Cmp::Position>();
  for ( auto [lc_entity, lc_cmp, lc_pos_cmp] : loot_container_view.each() )
  {
    if ( Utils::is_visible_in_view( RenderSystem::getGameView(), lc_pos_cmp ) )
    {
      SPDLOG_DEBUG( "Opened loot container at ({}, {})", lc_pos_cmp.position.x,
                    lc_pos_cmp.position.y );
      auto [sprite_type, sprite_index] = m_sprite_factory.get_random_type_and_texture_index(
          std::vector<std::string>{ "EXTRA_HEALTH", "EXTRA_BOMBS", "INFINI_BOMBS", "CHAIN_BOMBS",
                                    "WEAPON_BOOST" } );

      // clang-format off
      auto loot_entt = Factory::createLootDrop( 
        getReg(), 
        Cmp::SpriteAnimation( 0, 0, true, sprite_type, sprite_index ),                                        
        sf::FloatRect{ lc_pos_cmp.position, lc_pos_cmp.size }, 
        Sys::BaseSystem::IncludePack<>{},
        Sys::BaseSystem::ExcludePack<Cmp::PlayableCharacter, Cmp::ReservedPosition>{} );
      // clang-format on

      if ( loot_entt != entt::null ) { m_sound_bank.get_effect( "break_pot" ).play(); }

      Factory::destroyLootContainer( getReg(), lc_entity );
    }
  }

  m_altar_activation_clock.restart();
  return true;
}

} // namespace ProceduralMaze::Sys
