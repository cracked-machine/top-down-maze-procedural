#include "ShockwaveSystem.hpp"
#include <Components/NpcShockwave.hpp>
#include <Sprites/Shockwave.hpp>
#include <numbers>

namespace ProceduralMaze::Sys
{

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
  if ( point_angle < 0 ) point_angle += 2 * std::numbers::pi;

  // Check if this angle falls within any visible segment
  for ( const auto &segment : shockwave.sprite.getVisibleSegments() )
  {
    float start_angle = segment.getStartAngle();
    float end_angle = segment.getEndAngle();

    // Normalize angles to [0, 2π]
    while ( start_angle < 0 )
      start_angle += 2 * std::numbers::pi;
    while ( end_angle < 0 )
      end_angle += 2 * std::numbers::pi;
    while ( start_angle >= 2 * std::numbers::pi )
      start_angle -= 2 * std::numbers::pi;
    while ( end_angle >= 2 * std::numbers::pi )
      end_angle -= 2 * std::numbers::pi;

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

bool ShockwaveSystem::intersectsWithVisibleSegments( const Cmp::NpcShockwave &shockwave, const sf::FloatRect &rect )
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

      if ( rect.contains( inner_point ) || rect.contains( outer_point ) ) { return true; }
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

} // namespace ProceduralMaze::Sys