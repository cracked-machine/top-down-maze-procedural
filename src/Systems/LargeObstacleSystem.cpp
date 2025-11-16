#include <Components/GraveSegment.hpp>
#include <Components/LargeObstacle.hpp>
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

LargeObstacleSystem::LargeObstacleSystem( sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                                          Audio::SoundBank &sound_bank )
    : BaseSystem( window, sprite_factory, sound_bank )
{
  // The entt::dispatcher is independent of the registry, so it is safe to bind event handlers in the constructor
  std::ignore = getEventDispatcher().sink<Events::PlayerActionEvent>().connect<&LargeObstacleSystem::on_player_action>( this );
}

void LargeObstacleSystem::check_player_lo_collision( Events::PlayerActionEvent::GameActions action )
{
  if ( action != Events::PlayerActionEvent::GameActions::ACTIVATE ) return;

  auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::PlayerCandlesCount, Cmp::PlayerKeysCount>();
  auto large_obstacle_view = m_reg->view<Cmp::LargeObstacle>();

  for ( auto [pc_entity, pc_cmp, pc_pos_cmp, pc_candles_cmp, pc_keys_cmp] : player_view.each() )
  {
    // slightly larger hitbox for large obstacles because we want to trigger
    // collision when we get CLOSE to them
    auto player_hitbox = Cmp::RectBounds( pc_pos_cmp.position, kGridSquareSizePixelsF, 1.5f );

    for ( auto [lo_entity, lo_cmp] : large_obstacle_view.each() )
    {
      //   // only spawn one npc per large obstacle but give proportionate count for the grave size
      //   uint8_t activated_grave_count = 0;

      if ( player_hitbox.findIntersection( lo_cmp ) )
      {
        SPDLOG_DEBUG( "Player collided with LargeObstacle at ({}, {})", lo_cmp.position.x, lo_cmp.position.y );
        check_player_shrine_activation( lo_cmp, pc_candles_cmp );
        check_player_grave_activation( lo_cmp, lo_entity, pc_cmp );
      }
    }
  }
}

void LargeObstacleSystem::check_player_shrine_activation( Cmp::LargeObstacle &lo_cmp, Cmp::PlayerCandlesCount &pc_candles_cmp )
{

  uint8_t activated_shrine_segment_count = 0;
  auto shrine_view = m_reg->view<Cmp::ShrineSegment, Cmp::Position>();
  for ( auto [shrine_entity, shrine_cmp, shrine_pos_cmp] : shrine_view.each() )
  {
    // dont proceed if large obstacle powers are already active
    if ( lo_cmp.are_powers_active() ) break;

    // player doesn't have enough candles? - skip
    auto &shrine_cost = get_persistent_component<Cmp::Persistent::ShrineCostPerSprite>();
    if ( pc_candles_cmp.get_count() < shrine_cost.get_value() ) break;
    SPDLOG_DEBUG( "Player has enough candles ({}), proceeding with shrine check", pc_candles_cmp.get_count() );

    // is shrine part of the collided large obstacle?
    if ( not shrine_pos_cmp.findIntersection( lo_cmp ) ) continue;
    SPDLOG_DEBUG( "Found shrine at ({}, {}) for activation check", shrine_pos_cmp.position.x, shrine_pos_cmp.position.y );

    // Convert pixel size to grid size, then calculate threshold
    auto lo_grid_size = lo_cmp.size.componentWiseDiv( kGridSquareSizePixelsF );
    auto lo_count_threshold = ( lo_grid_size.x * lo_grid_size.y );

    if ( lo_cmp.get_activated_sprite_count() < lo_count_threshold )
    {
      SPDLOG_DEBUG( "Activating shrine sprite {}/{}.", lo_cmp.get_activated_sprite_count() + 1, lo_count_threshold );
      // activate the next shrine candle and switch to the new animation sequence

      switch ( lo_cmp.get_activated_sprite_count() )
      {
        case 0:
          // clang-format off
          m_reg->patch<Cmp::SpriteAnimation>( shrine_entity, [&]( Cmp::SpriteAnimation &anim_cmp ) { anim_cmp.m_sprite_type = "SHRINE.one"; } );
          // clang-format on
          activated_shrine_segment_count++;
          SPDLOG_DEBUG( "Shrine activated to state ONE." );
          break;
        case 1:
          // clang-format off
          m_reg->patch<Cmp::SpriteAnimation>( shrine_entity, [&]( Cmp::SpriteAnimation &anim_cmp ) { anim_cmp.m_sprite_type = "SHRINE.two"; } );
          // clang-format on
          activated_shrine_segment_count++;
          SPDLOG_DEBUG( "Shrine activated to state TWO." );
          break;

        case 2:
          // clang-format off
          m_reg->patch<Cmp::SpriteAnimation>( shrine_entity, [&]( Cmp::SpriteAnimation &anim_cmp ) { anim_cmp.m_sprite_type = "SHRINE.three"; } );
          // clang-format on
          activated_shrine_segment_count++;
          SPDLOG_DEBUG( "Shrine activated to state THREE." );
          break;
        case 3:
          // clang-format off
          m_reg->patch<Cmp::SpriteAnimation>( shrine_entity, [&]( Cmp::SpriteAnimation &anim_cmp ) { anim_cmp.m_sprite_type = "SHRINE.four"; } );
          // clang-format on
          activated_shrine_segment_count++;
          SPDLOG_DEBUG( "Shrine activated to state FOUR." );
          break;
        default:
          break;
      }
    }
    // we just incremented the state of each shrine segment so tax the player candles and mark the shrine as activated
    if ( activated_shrine_segment_count >= 4 )
    {
      lo_cmp.increment_activated_sprite_count();
      pc_candles_cmp.decrement_count( shrine_cost.get_value() );
      if ( lo_cmp.get_activated_sprite_count() >= lo_count_threshold ) { lo_cmp.set_powers_active(); }
    }

    // Did we just fully activate the shrine?
    if ( lo_cmp.are_powers_active() )
    {
      SPDLOG_DEBUG( "Shrine fully activated!" );
      m_sound_bank.get_effect( "shrine_lighting" ).play();
      // drop the key loot
      auto lo_cmp_bounds = Cmp::RectBounds( lo_cmp.position, lo_cmp.size, 1.5f );
      // clang-format off
              auto obst_entity = create_loot_drop( 
                Cmp::Loot{ "KEY_DROP", 0 },
                sf::FloatRect{ lo_cmp_bounds.position(), 
                lo_cmp_bounds.size() }, 
                IncludePack<>{},
                ExcludePack<>{} 
              );
      // clang-format on

      if ( obst_entity != entt::null ) { m_sound_bank.get_effect( "drop_loot" ).play(); }
      m_shrine_activation_clock.restart();
      break;
    }
  }

  if ( lo_cmp.are_powers_active() )
  {
    for ( auto [shrine_entity, shrine_cmp, shrine_pos_cmp] : shrine_view.each() )
    {
      SPDLOG_DEBUG( "Checking for special power activation." );
      auto anim_sprite_cmp = m_reg->try_get<Cmp::SpriteAnimation>( shrine_entity );
      if ( anim_sprite_cmp && m_shrine_activation_clock.getElapsedTime() > m_shrine_activation_cooldown )
      {
        activate_shrine_special_power();
      }
    }
  }
}

void LargeObstacleSystem::check_player_grave_activation( Cmp::LargeObstacle &lo_cmp, entt::entity &lo_entity,
                                                         Cmp::PlayableCharacter &pc_cmp )
{
  // only spawn one npc per large obstacle but give proportionate count for the grave size
  // uint8_t activated_grave_segments_count = 0;

  // check for grave collisions
  auto grave_view = m_reg->view<Cmp::GraveSegment, Cmp::Position, Cmp::SpriteAnimation>();
  for ( auto [grave_entity, grave_cmp, grave_pos_cmp, grave_anim_cmp] : grave_view.each() )
  {
    // dont proceed if large obstacle powers are already active
    if ( lo_cmp.are_powers_active() ) break;

    // is grave part of the collided large obstacle?
    if ( not lo_cmp.findIntersection( grave_pos_cmp ) ) continue;
    SPDLOG_DEBUG( "Found grave at ({}, {}) for activation check", grave_pos_cmp.position.x, grave_pos_cmp.position.y );

    // have we activated all the parts of the grave yet?
    auto &grave_ms = m_sprite_factory.get_multisprite_by_type( grave_anim_cmp.m_sprite_type );
    auto activation_threshold = grave_ms.get_grid_size().width * grave_ms.get_grid_size().height;
    SPDLOG_DEBUG( "Grave activation threshold: {}/{}", lo_cmp.get_activated_sprite_count(), activation_threshold );

    // if ( lo_cmp.get_activated_sprite_count() >= activation_threshold ) continue;
    if ( lo_cmp.get_activated_sprite_count() < activation_threshold )
    {
      SPDLOG_DEBUG( "Activating grave sprite {}/{}.", lo_cmp.get_activated_sprite_count() + 1, activation_threshold );
      lo_cmp.increment_activated_sprite_count();
      if ( std::string::size_type n = grave_anim_cmp.m_sprite_type.find( "." ); n != std::string::npos )
      {
        grave_anim_cmp.m_sprite_type = grave_anim_cmp.m_sprite_type.substr( 0, n ) + ".opened";
        SPDLOG_DEBUG( "Grave Cmp::SpriteAnimation changed to opened type: {}", grave_anim_cmp.m_sprite_type );
      }
    }

    if ( lo_cmp.get_activated_sprite_count() >= activation_threshold )
    {
      SPDLOG_DEBUG( "Activating large obstacle at ({}, {})", lo_cmp.position.x, lo_cmp.position.y );
      lo_cmp.set_powers_active( true );
    }
    // choose a random consequence for activating graves: spawn npc, drop bomb, give candles
    if ( lo_cmp.are_powers_active() )
    {
      auto grave_activation_rng = Cmp::RandomInt( 1, 3 );
      auto consequence = grave_activation_rng.gen();
      switch ( consequence )
      {
        case 1:
          SPDLOG_INFO( "Grave activated NPC trap." );
          getEventDispatcher().trigger( Events::NpcCreationEvent( lo_entity, "NPCGHOST" ) );
          break;
        case 2:
          SPDLOG_INFO( "Grave activated bomb trap." );
          pc_cmp.bomb_inventory += 1;
          getEventDispatcher().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::GRAVE_BOMB ) );
          break;
        case 3:

          auto lo_cmp_bounds = Cmp::RectBounds( lo_cmp.position, lo_cmp.size, 1.5f );
          // clang-format off
              auto obst_entity = create_loot_drop( 
                Cmp::Loot{ "CANDLE_DROP", 0 },
                sf::FloatRect{ lo_cmp_bounds.position(), 
                lo_cmp_bounds.size() }, 
                IncludePack<>{},
                ExcludePack<>{} 
              );
          // clang-format on

          if ( obst_entity != entt::null ) { m_sound_bank.get_effect( "drop_loot" ).play(); }
          break;
      }
    }
  }
}

bool LargeObstacleSystem::activate_shrine_special_power()
{

  for ( auto [pc_entity, pc_relic_count_cmp] : m_reg->view<Cmp::PlayerRelicCount>().each() )
  {

    if ( pc_relic_count_cmp.get_count() < 1 ) return false;

    SPDLOG_DEBUG( "Activating shrine special power!" );
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
        auto npc_view = m_reg->view<Cmp::NPC, Cmp::Position>();
        for ( auto [npc_entity, npc_cmp, npc_pos_cmp] : npc_view.each() )
        {
          if ( is_visible_in_view( RenderSystem::getGameView(), npc_pos_cmp ) )
          {
            SPDLOG_INFO( "Killed NPC at ({}, {})", npc_pos_cmp.position.x, npc_pos_cmp.position.y );
            getEventDispatcher().trigger( Events::NpcDeathEvent( npc_entity ) );
          }
        }
        break;
      }
      case 2:
      {
        SPDLOG_INFO( "Special Power: Re-enable all nearby obstacles!" );
        auto obstacle_view = m_reg->view<Cmp::Obstacle, Cmp::Position>();
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
        auto loot_container_view = m_reg->view<Cmp::LootContainer, Cmp::Position>();
        for ( auto [lc_entity, lc_cmp, lc_pos_cmp] : loot_container_view.each() )
        {
          if ( is_visible_in_view( RenderSystem::getGameView(), lc_pos_cmp ) )
          {
            SPDLOG_INFO( "Opened loot container at ({}, {})", lc_pos_cmp.position.x, lc_pos_cmp.position.y );
            getEventDispatcher().trigger( Events::LootContainerDestroyedEvent( lc_entity ) );
          }
        }
        break;
      }
    }
  }
  return true;
}

} // namespace ProceduralMaze::Sys