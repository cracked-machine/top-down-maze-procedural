#include <Components/WeaponLevel.hpp>
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
#include <Components/PlayerScore.hpp>
#include <Components/Position.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Events/AnimDirectionChangeEvent.hpp>
#include <Events/AnimResetFrameEvent.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/PlayerSystem.hpp>

namespace ProceduralMaze::Sys {

PlayerSystem::PlayerSystem( ProceduralMaze::SharedEnttRegistry registry, sf::RenderWindow &window,
                            Sprites::SpriteFactory &sprite_factory )
    : BaseSystem( registry, window, sprite_factory )
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
  m_reg->emplace<Cmp::PlayerScore>( entity, 0 );
  m_reg->emplace<Cmp::WeaponLevel>( entity, 100.f );
}

void PlayerSystem::update_movement( sf::Time deltaTime, bool skip_collision_check )
{
  const float dt = deltaTime.asSeconds();

  auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Direction, Cmp::PCDetectionBounds>();
  for ( auto [entity, pc_cmp, pos_cmp, dir_cmp, pc_detection_bounds] : player_view.each() )
  {
    auto lerp_cmp = m_reg->try_get<Cmp::LerpPosition>( entity );
    bool wants_to_move = dir_cmp != sf::Vector2f( 0.0f, 0.0f );

    // Check if moving diagonally
    bool is_diagonal = ( dir_cmp.x != 0.0f ) && ( dir_cmp.y != 0.0f );
    bool diagonal_between_obstacles = is_diagonal && isDiagonalMovementBetweenObstacles( pos_cmp, dir_cmp );

    // Only start new movement when not lerping
    if ( wants_to_move && !lerp_cmp )
    {
      // make a copy to determine if new target position is valid
      sf::FloatRect new_pos{ pos_cmp };
      new_pos.position.x = pos_cmp.position.x + ( dir_cmp.x * BaseSystem::kGridSquareSizePixels.x );
      new_pos.position.y = pos_cmp.position.y + ( dir_cmp.y * BaseSystem::kGridSquareSizePixels.y );

      if ( is_valid_move( new_pos ) || skip_collision_check )
      {
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
    }

    // now we modify... ongoing lerp movement
    if ( lerp_cmp && lerp_cmp->m_lerp_factor < 1.0f )
    {
      lerp_cmp->m_lerp_factor += ( lerp_cmp->m_lerp_speed * dt );
      lerp_cmp->m_lerp_factor = std::min( lerp_cmp->m_lerp_factor, 1.0f );

      // interpolate to the new position
      pos_cmp.position.x = std::lerp( lerp_cmp->m_start.x, lerp_cmp->m_target.x, lerp_cmp->m_lerp_factor );
      pos_cmp.position.y = std::lerp( lerp_cmp->m_start.y, lerp_cmp->m_target.y, lerp_cmp->m_lerp_factor );

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

    // Animation events
    if ( dir_cmp == sf::Vector2f( 0.0f, 0.0f ) ) { getEventDispatcher().trigger( Events::AnimResetFrameEvent( entity ) ); }
    else { getEventDispatcher().trigger( Events::AnimDirectionChangeEvent( entity ) ); }
  }
}

} // namespace ProceduralMaze::Sys