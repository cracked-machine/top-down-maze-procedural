#include <Direction.hpp>
#include <Events/AnimDirectionChangeEvent.hpp>
#include <Events/AnimResetFrameEvent.hpp>
#include <LerpPosition.hpp>
#include <Obstacle.hpp>
#include <Persistent/PlayerDiagonalLerpSpeedModifier.hpp>
#include <Persistent/PlayerLerpSpeed.hpp>
#include <Persistent/PlayerShortcutLerpSpeedModifier.hpp>
#include <Persistent/PlayerStartPosition.hpp>
#include <Persistent/PlayerSubmergedlLerpSpeedModifier.hpp>
#include <PlayableCharacter.hpp>
#include <PlayerScore.hpp>
#include <PlayerSystem.hpp>
#include <Position.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SpriteAnimation.hpp>
#include <SpriteFactory.hpp>

namespace ProceduralMaze::Sys {

PlayerSystem::PlayerSystem( ProceduralMaze::SharedEnttRegistry registry )
    : BaseSystem( registry )
{
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
  m_reg->emplace<Cmp::Position>( entity, start_pos );

  auto &bomb_inventory = get_persistent_component<Cmp::Persistent::BombInventory>();
  auto &blast_radius = get_persistent_component<Cmp::Persistent::BlastRadius>();
  m_reg->emplace<Cmp::PlayableCharacter>( entity, bomb_inventory(), blast_radius() );

  m_reg->emplace<Cmp::Direction>( entity, sf::Vector2f{ 0, 0 } );

  auto &pc_detection_scale = get_persistent_component<Cmp::Persistent::PlayerDetectionScale>();

  m_reg->emplace<Cmp::PCDetectionBounds>( entity, start_pos, sf::Vector2f{ BaseSystem::kGridSquareSizePixels },
                                          pc_detection_scale() );

  m_reg->emplace<Cmp::SpriteAnimation>( entity );
  m_reg->emplace<Cmp::PlayerScore>( entity, 0 );
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
      auto target_pos = sf::Vector2f{ pos_cmp.x + ( dir_cmp.x * BaseSystem::kGridSquareSizePixels.x ),
                                      pos_cmp.y + ( dir_cmp.y * BaseSystem::kGridSquareSizePixels.y ) };

      if ( is_valid_move( target_pos ) || skip_collision_check )
      {
        auto &player_lerp_speed = get_persistent_component<Cmp::Persistent::PlayerLerpSpeed>();
        auto &
            diagonal_lerp_speed_modifier = get_persistent_component<Cmp::Persistent::PlayerDiagonalLerpSpeedModifier>();
        auto &
            shortcut_lerp_speed_modifier = get_persistent_component<Cmp::Persistent::PlayerShortcutLerpSpeedModifier>();
        auto &submerged_lerp_speed_modifier = get_persistent_component<
            Cmp::Persistent::PlayerSubmergedLerpSpeedModifier>();

        float speed_modifier = 1.0f;
        if ( diagonal_between_obstacles )
        {
          // Check if shortcut movement is disabled (speed modifier at or near zero)
          if ( shortcut_lerp_speed_modifier() < 0.01f )
          {
            // Block this movement entirely instead of making it super slow
            continue; // Skip to next entity, don't start the movement
          }

          // Extra slow when squeezing between obstacles
          speed_modifier = shortcut_lerp_speed_modifier();
        }
        else if ( is_diagonal )
        {
          // Normal diagonal slowdown
          speed_modifier = diagonal_lerp_speed_modifier();
        }

        float adjusted_speed = player_lerp_speed() * speed_modifier;
        // add additional modifier if player is underwater
        if ( pc_cmp.underwater ) { adjusted_speed *= submerged_lerp_speed_modifier(); }

        m_reg->emplace<Cmp::LerpPosition>( entity, target_pos, adjusted_speed );
        lerp_cmp = m_reg->try_get<Cmp::LerpPosition>( entity );

        lerp_cmp->m_start = pos_cmp;
        lerp_cmp->m_target = target_pos;
        lerp_cmp->m_lerp_factor = 0.0f;
      }
    }

    // Update existing lerp
    if ( lerp_cmp && lerp_cmp->m_lerp_factor < 1.0f )
    {

      lerp_cmp->m_lerp_factor += ( lerp_cmp->m_lerp_speed * dt );
      lerp_cmp->m_lerp_factor = std::min( lerp_cmp->m_lerp_factor, 1.0f );

      // Interpolate position
      pos_cmp.x = std::lerp( lerp_cmp->m_start.x, lerp_cmp->m_target.x, lerp_cmp->m_lerp_factor );
      pos_cmp.y = std::lerp( lerp_cmp->m_start.y, lerp_cmp->m_target.y, lerp_cmp->m_lerp_factor );

      // Update detection bounds during lerp
      pc_detection_bounds.position( pos_cmp );

      // Complete lerp
      if ( lerp_cmp->m_lerp_factor >= 1.0f )
      {
        pos_cmp = lerp_cmp->m_target;
        pc_detection_bounds.position( pos_cmp );
        m_reg->remove<Cmp::LerpPosition>( entity );
      }
    }

    // Animation events
    if ( dir_cmp == sf::Vector2f( 0.0f, 0.0f ) )
    {
      getEventDispatcher().trigger( Events::AnimResetFrameEvent( entity ) );
    }
    else { getEventDispatcher().trigger( Events::AnimDirectionChangeEvent( entity ) ); }
  }
}

} // namespace ProceduralMaze::Sys