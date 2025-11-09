#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/PlayerHealth.hpp>
#include <Components/PlayerKeysCount.hpp>
#include <Components/PlayerMortality.hpp>
#include <Components/PlayerRelicCount.hpp>
#include <Components/WeaponLevel.hpp>
#include <Components/WormholeJump.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <Components/Direction.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/PlayerDiagonalLerpSpeedModifier.hpp>
#include <Components/Persistent/PlayerLerpSpeed.hpp>
#include <Components/Persistent/PlayerShortcutLerpSpeedModifier.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Persistent/PlayerSubmergedLerpSpeedModifier.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerCandlesCount.hpp>
#include <Components/Position.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Events/AnimDirectionChangeEvent.hpp>
#include <Events/AnimResetFrameEvent.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/PlayerSystem.hpp>

namespace ProceduralMaze::Sys {

PlayerSystem::PlayerSystem( ProceduralMaze::SharedEnttRegistry registry, sf::RenderWindow &window,
                            Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( registry, window, sprite_factory, sound_bank )
{
  SPDLOG_DEBUG( "PlayerSystem initialized" );
}

void PlayerSystem::add_player_entity()
{
  SPDLOG_INFO( "Creating player entity" );
  auto entity = m_reg->create();

  // start position must be pixel coordinates within the screen resolution (kDisplaySize),
  // but also grid aligned (kMapGridSize) to avoid collision detection errors.
  // So we must recalc start position to the nearest grid position here
  auto start_pos = get_persistent_component<Cmp::Persistent::PlayerStartPosition>();
  start_pos = snap_to_grid( start_pos );
  m_reg->emplace<Cmp::Position>( entity, start_pos, kGridSquareSizePixelsF );

  auto &bomb_inventory = get_persistent_component<Cmp::Persistent::BombInventory>();
  auto &blast_radius = get_persistent_component<Cmp::Persistent::BlastRadius>();
  m_reg->emplace<Cmp::PlayableCharacter>( entity, bomb_inventory.get_value(), blast_radius.get_value() );

  m_reg->emplace<Cmp::Direction>( entity, sf::Vector2f{ 0, 0 } );

  auto &pc_detection_scale = get_persistent_component<Cmp::Persistent::PlayerDetectionScale>();

  m_reg->emplace<Cmp::PCDetectionBounds>( entity, start_pos, kGridSquareSizePixelsF, pc_detection_scale.get_value() );

  m_reg->emplace<Cmp::SpriteAnimation>( entity );
  m_reg->emplace<Cmp::PlayerCandlesCount>( entity, 0 );
  m_reg->emplace<Cmp::PlayerKeysCount>( entity, 0 );
  m_reg->emplace<Cmp::PlayerRelicCount>( entity, 0 );
  m_reg->emplace<Cmp::PlayerHealth>( entity, 100 );
  m_reg->emplace<Cmp::PlayerMortality>( entity, Cmp::PlayerMortality::State::ALIVE );
  m_reg->emplace<Cmp::WeaponLevel>( entity, 100.f );
}

Cmp::PlayerMortality::State PlayerSystem::check_player_mortality()
{
  auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::PlayerHealth, Cmp::PlayerMortality>();
  Cmp::PlayerMortality::State mortality_state = Cmp::PlayerMortality::State::ALIVE;
  for ( auto [entity, pc_cmp, health_cmp, mortality_cmp] : player_view.each() )
  {

    if ( mortality_cmp.state == Cmp::PlayerMortality::State::DEAD )
    {
      mortality_state = mortality_cmp.state;
      break;
    }
    if ( health_cmp.health <= 0 && mortality_cmp.death_progress >= 1.0f )
    {
      mortality_cmp.state = Cmp::PlayerMortality::State::DEAD;
      SPDLOG_INFO( "Player has progressed to deadness." );
    }
    mortality_state = mortality_cmp.state;
  }
  return mortality_state;
}

void PlayerSystem::update_movement( sf::Time globalDeltaTime, bool skip_collision_check )
{
  const float dt = globalDeltaTime.asSeconds();

  auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Direction, Cmp::PCDetectionBounds>();
  for ( auto [entity, pc_cmp, pos_cmp, dir_cmp, pc_detection_bounds] : player_view.each() )
  {

    auto lerp_cmp = m_reg->try_get<Cmp::LerpPosition>( entity );
    bool wants_to_move = dir_cmp != sf::Vector2f( 0.0f, 0.0f );

    // Animation events
    if ( dir_cmp == sf::Vector2f( 0.0f, 0.0f ) ) { getEventDispatcher().trigger( Events::AnimResetFrameEvent( entity ) ); }
    else
    {
      // prevent spamming direction change events during wormhole jumps
      if ( !m_reg->try_get<Cmp::WormholeJump>( entity ) )
      {
        getEventDispatcher().trigger( Events::AnimDirectionChangeEvent( entity ) );
      }
    }

    // Only start new movement when not lerping
    if ( wants_to_move && !lerp_cmp )
    {
      // disable new lerp starting during wormhole jump
      if ( m_reg->try_get<Cmp::WormholeJump>( entity ) ) return;

      // make a copy to determine if new target position is valid
      sf::FloatRect new_pos{ pos_cmp };
      new_pos.position.x = pos_cmp.position.x + ( dir_cmp.x * BaseSystem::kGridSquareSizePixels.x );
      new_pos.position.y = pos_cmp.position.y + ( dir_cmp.y * BaseSystem::kGridSquareSizePixels.y );

      // Check collision ONCE
      bool can_move = skip_collision_check || is_valid_move( new_pos );
      if ( !can_move ) continue; // Early exit if blocked

      // Check if moving diagonally AFTER we know movement is valid
      bool is_diagonal = ( dir_cmp.x != 0.0f ) && ( dir_cmp.y != 0.0f );
      bool diagonal_between_obstacles = is_diagonal && isDiagonalMovementBetweenObstacles( pos_cmp, dir_cmp );

      auto &player_lerp_speed = get_persistent_component<Cmp::Persistent::PlayerLerpSpeed>();
      auto &diagonal_lerp_speed_modifier = get_persistent_component<Cmp::Persistent::PlayerDiagonalLerpSpeedModifier>();
      auto &shortcut_lerp_speed_modifier = get_persistent_component<Cmp::Persistent::PlayerShortcutLerpSpeedModifier>();
      auto &submerged_lerp_speed_modifier = get_persistent_component<Cmp::Persistent::PlayerSubmergedLerpSpeedModifier>();

      float speed_modifier = 1.0f;
      if ( diagonal_between_obstacles )
      {
        // Check if shortcut movement is disabled (speed modifier at or near zero)
        if ( shortcut_lerp_speed_modifier.get_value() < 0.01f )
        {
          // Block this movement entirely instead of making it super slow
          continue; // Skip to next entity, don't start the movement
        }

        // Extra slow when squeezing between obstacles
        speed_modifier = shortcut_lerp_speed_modifier.get_value();
      }
      else if ( is_diagonal )
      {
        // Normal diagonal slowdown
        speed_modifier = diagonal_lerp_speed_modifier.get_value();
      }

      float adjusted_speed = player_lerp_speed.get_value() * speed_modifier;
      // add additional modifier if player is underwater
      if ( pc_cmp.underwater ) { adjusted_speed *= submerged_lerp_speed_modifier.get_value(); }

      m_reg->emplace<Cmp::LerpPosition>( entity, new_pos.position, adjusted_speed );
      lerp_cmp = m_reg->try_get<Cmp::LerpPosition>( entity );

      lerp_cmp->m_start = pos_cmp.position;
      lerp_cmp->m_target = new_pos.position;
      lerp_cmp->m_lerp_factor = 0.0f;
    }

    // now we modify... ongoing lerp movement
    if ( lerp_cmp && lerp_cmp->m_lerp_factor < 1.0f )
    {
      lerp_cmp->m_lerp_factor += ( lerp_cmp->m_lerp_speed * dt );
      lerp_cmp->m_lerp_factor = std::min( lerp_cmp->m_lerp_factor, 1.0f );

      // interpolate to the new position
      const float t = lerp_cmp->m_lerp_factor;
      const float one_minus_t = 1.0f - t;

      // Simple manual lerp - 33 lines of assembly vs 134 for std::lerp vs 54 for std::fma
      // https://godbolt.org/z/YdeKco5d6
      pos_cmp.position.x = one_minus_t * lerp_cmp->m_start.x + t * lerp_cmp->m_target.x;
      pos_cmp.position.y = one_minus_t * lerp_cmp->m_start.y + t * lerp_cmp->m_target.y;

      // Update detection bounds position during lerp to keep in sync
      pc_detection_bounds.position( pos_cmp.position );

      // if lerp is complete, finalize position
      if ( lerp_cmp->m_lerp_factor >= 1.0f )
      {
        pos_cmp.position = lerp_cmp->m_target;
        pc_detection_bounds.position( pos_cmp.position );
        m_reg->remove<Cmp::LerpPosition>( entity );
      }
    }
  }
}

void PlayerSystem::play_footsteps_sound()
{
  // Restarting prematurely creates a stutter effect, so check first
  auto &footsteps = m_sound_bank.get_effect( "footsteps" );
  if ( footsteps.getStatus() == sf::Sound::Status::Playing ) return;
  footsteps.play();
}

void PlayerSystem::stop_footsteps_sound() { m_sound_bank.get_effect( "footsteps" ).stop(); }

} // namespace ProceduralMaze::Sys