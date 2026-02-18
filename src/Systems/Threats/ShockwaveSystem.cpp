#include "ShockwaveSystem.hpp"
#include <Audio/SoundBank.hpp>
#include <Components/Npc/NpcShockwave.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerHealth.hpp>
#include <Components/Player/PlayerMortality.hpp>
#include <Events/PlayerMortalityEvent.hpp>
#include <Sprites/Shockwave.hpp>
#include <Systems/PersistSystem.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Utils.hpp>

namespace ProceduralMaze::Sys
{

ShockwaveSystem::ShockwaveSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                                  Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
}

Sprites::Shockwave::CircleSegments ShockwaveSystem::splitSegmentByObstacle( const Sprites::CircleSegment &segment, const sf::FloatRect &obstacle_rect,
                                                                            sf::Vector2f shockwave_position, float radius, const int samples )
{
  Sprites::Shockwave::CircleSegments result;

  // Sample points along the segment to find intersections
  std::vector<bool> intersections( samples, false );

  float angle_range = segment.getEndAngle() - segment.getStartAngle();

  for ( int i = 0; i < samples; ++i )
  {
    float t = static_cast<float>( i ) / ( samples - 1 );
    float angle = segment.getStartAngle() + t * angle_range;

    sf::Vector2f point = shockwave_position + sf::Vector2f( std::cos( angle ) * radius, std::sin( angle ) * radius );
    intersections[i] = obstacle_rect.contains( point );
  }

  // Find continuous non-intersecting ranges
  int start_idx = -1;
  for ( int i = 0; i < samples; ++i )
  {
    if ( !intersections[i] && start_idx == -1 )
    {
      start_idx = i; // Start of non-intersecting segment
    }
    else if ( intersections[i] && start_idx != -1 )
    {
      // End of non-intersecting segment
      float start_angle = segment.getStartAngle() + ( static_cast<float>( start_idx ) / ( samples - 1 ) ) * angle_range;
      float end_angle = segment.getStartAngle() + ( static_cast<float>( i - 1 ) / ( samples - 1 ) ) * angle_range;

      if ( end_angle > start_angle ) { result.emplace_back( start_angle, end_angle, true ); }
      start_idx = -1;
    }
  }

  // Handle case where segment ends with non-intersecting part
  if ( start_idx != -1 )
  {
    float start_angle = segment.getStartAngle() + ( static_cast<float>( start_idx ) / ( samples - 1 ) ) * angle_range;
    result.emplace_back( start_angle, segment.getEndAngle(), true );
  }

  return result;
}

bool ShockwaveSystem::pointIntersectsVisibleSegments( const Cmp::NpcShockwave &shockwave, sf::Vector2f point )
{
  sf::Vector2f position = shockwave.sprite.getPosition();
  float radius = shockwave.sprite.getRadius();
  float outline_thickness = shockwave.sprite.getOutlineThickness();

  float distance = std::sqrt( std::pow( point.x - position.x, 2 ) + std::pow( point.y - position.y, 2 ) );

  // Check if point is at the right distance from center
  if ( std::abs( distance - radius ) > outline_thickness / 2.0f ) { return false; }

  // Calculate angle of the point relative to center
  float point_angle = std::atan2( point.y - position.y, point.x - position.x );
  point_angle = Utils::Maths::normalizeAngle( point_angle );

  // Check if this angle falls within any visible segment
  for ( const auto &segment : shockwave.sprite.getVisibleSegments() )
  {
    float start_angle = Utils::Maths::normalizeAngle( segment.getStartAngle() );
    float end_angle = Utils::Maths::normalizeAngle( segment.getEndAngle() );

    // Handle wrap-around case
    if ( start_angle <= end_angle )
    {
      if ( point_angle >= start_angle && point_angle <= end_angle ) { return true; }
    }
    else
    {
      // Segment wraps around 0
      if ( point_angle >= start_angle || point_angle <= end_angle ) { return true; }
    }
  }

  return false;
}

bool ShockwaveSystem::intersectsWithVisibleSegments( entt::registry &reg, const Cmp::NpcShockwave &shockwave, const sf::FloatRect &rect )
{
  sf::Vector2f position = shockwave.sprite.getPosition();
  float radius = shockwave.sprite.getRadius();
  float outline_thickness = shockwave.sprite.getOutlineThickness();
  int points_per_segment = shockwave.sprite.getPointsPerSegment();

  for ( const auto &segment : shockwave.sprite.getVisibleSegments() )
  {
    float angle_range = segment.getEndAngle() - segment.getStartAngle();

    for ( int i = 0; i < points_per_segment; ++i )
    {
      float t = static_cast<float>( i ) / ( points_per_segment - 1 );
      float angle = segment.getStartAngle() + t * angle_range;

      // Check both inner and outer radius points to account for thickness
      float inner_radius = radius - outline_thickness / 2.0f;
      float outer_radius = radius + outline_thickness / 2.0f;

      sf::Vector2f inner_point = position + sf::Vector2f( std::cos( angle ) * inner_radius, std::sin( angle ) * inner_radius );
      sf::Vector2f outer_point = position + sf::Vector2f( std::cos( angle ) * outer_radius, std::sin( angle ) * outer_radius );

      if ( rect.contains( inner_point ) || rect.contains( outer_point ) )
      {
        // do shockwave/player knockback
        sf::Vector2f shockwave_direction( std::cos( angle ), std::sin( angle ) );
        shockwave_direction = shockwave_direction.normalized();

        auto &player_pos_cmp = Utils::get_player_position( reg );
        auto new_position = Utils::snap_to_grid( player_pos_cmp.position +
                                                 ( shockwave_direction.componentWiseMul( Constants::kGridSquareSizePixelsF ) ) );
        SPDLOG_DEBUG( "Player position was {},{} - Knockback direction is {}, {} - New Position should be {},{}", player_pos_cmp.position.x,
                      player_pos_cmp.position.y, normalised_direction.x, normalised_direction.y, new_position.x, new_position.y );

        // make sure player isnt knocked into an obstacle
        bool is_valid = true;
        for ( auto [obstacle_entt, obstacle_cmp, obstacle_pos_cmp] : reg.view<Cmp::Obstacle, Cmp::Position>().each() )
        {
          if ( sf::FloatRect( new_position, Constants::kGridSquareSizePixelsF ).findIntersection( obstacle_pos_cmp ) ) is_valid = false;
        }
        if ( is_valid ) { player_pos_cmp.position = new_position; }
        else { SPDLOG_DEBUG( "New Position was invalid so cancelled" ); }

        return true;
      }
    }
  }
  return false;
}

// Remove segments that intersect with the given rectangle
void ShockwaveSystem::removeIntersectingSegments( const sf::FloatRect &obstacle_rect, Cmp::NpcShockwave &shockwave )
{
  Sprites::Shockwave::CircleSegments new_segments;

  for ( const auto &segment : shockwave.sprite.getSegments() )
  {
    if ( not segment.isVisible() ) continue;

    Sprites::Shockwave::CircleSegments non_intersecting = Sys::ShockwaveSystem::splitSegmentByObstacle(
        segment, obstacle_rect, shockwave.sprite.getPosition(), shockwave.sprite.getRadius(), shockwave.sprite.getPointsPerSegment() );
    new_segments.insert( new_segments.end(), non_intersecting.begin(), non_intersecting.end() );
  }

  shockwave.sprite.setSegments( std::move( new_segments ) );
}

void ShockwaveSystem::checkShockwavePlayerCollision()
{
  for ( auto entt : getReg().view<Cmp::NpcShockwave>() )
  {
    Cmp::NpcShockwave &shockwave = getReg().get<Cmp::NpcShockwave>( entt );
    auto &pc_damage_cooldown = Sys::PersistSystem::get<Cmp::Persist::PcDamageDelay>( getReg() );
    auto player_view = getReg().view<Cmp::PlayerCharacter, Cmp::Position, Cmp::PlayerHealth, Cmp::PlayerMortality>();

    for ( auto [player_entity, player_cmp, player_pos, player_health, player_mort_cmp] : player_view.each() )
    {
      // dont spam death events if the player is already dead
      if ( player_mort_cmp.state == Cmp::PlayerMortality::State::DEAD ) continue;
      if ( player_cmp.m_damage_cooldown_timer.getElapsedTime().asSeconds() < pc_damage_cooldown.get_value() ) continue;
      if ( Sys::ShockwaveSystem::intersectsWithVisibleSegments( getReg(), shockwave, player_pos ) )
      {
        player_health.health -= 10;
        m_sound_bank.get_effect( "damage_player" ).play();
        player_cmp.m_damage_cooldown_timer.restart();
        SPDLOG_INFO( "Player (health:{}) INTERSECTS with Shockwave (position: {},{} - effective_radius: {})", player_health.health,
                     shockwave.sprite.getPosition().x, shockwave.sprite.getPosition().y, shockwave.sprite.getRadius() );

        // trigger death animation
        if ( player_health.health <= 0 )
        {
          get_systems_event_queue().enqueue( Events::PlayerMortalityEvent( Cmp::PlayerMortality::State::SHOCKED, player_pos ) );
        }
      }
      else { SPDLOG_DEBUG( "Player does NOT intersect with shockwave (effective_radius: {})", shockwave.sprite.getRadius() ); }
    }
  }
}

} // namespace ProceduralMaze::Sys