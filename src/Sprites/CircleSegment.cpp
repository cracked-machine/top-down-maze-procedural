#include <SFML/Graphics/RenderTarget.hpp>
#include <Sprites/CircleSegment.hpp>
#include <algorithm>
#include <cmath>
#include <numbers>

namespace ProceduralMaze::Sprites
{

sf::FloatRect CircleSegment::getBounds( sf::Vector2f center, float radius, float outline_thickness ) const
{
  // Calculate the bounding box for this angular segment
  float inner_radius = radius - outline_thickness / 2.0f;
  float outer_radius = radius + outline_thickness / 2.0f;

  // Sample a few points along the arc to find min/max bounds
  constexpr int sample_points = 8;
  float min_x = std::numeric_limits<float>::max();
  float min_y = std::numeric_limits<float>::max();
  float max_x = std::numeric_limits<float>::lowest();
  float max_y = std::numeric_limits<float>::lowest();

  float angle_range = end_angle - start_angle;

  for ( int i = 0; i <= sample_points; ++i )
  {
    float t = static_cast<float>( i ) / sample_points;
    float angle = start_angle + t * angle_range;

    // Check both inner and outer radius points
    sf::Vector2f inner_point = center + sf::Vector2f( std::cos( angle ) * inner_radius, std::sin( angle ) * inner_radius );
    sf::Vector2f outer_point = center + sf::Vector2f( std::cos( angle ) * outer_radius, std::sin( angle ) * outer_radius );

    min_x = std::min( { min_x, inner_point.x, outer_point.x } );
    min_y = std::min( { min_y, inner_point.y, outer_point.y } );
    max_x = std::max( { max_x, inner_point.x, outer_point.x } );
    max_y = std::max( { max_y, inner_point.y, outer_point.y } );
  }

  return sf::FloatRect( { min_x, min_y }, { max_x - min_x, max_y - min_y } );
}

void CircleSegment::draw( sf::RenderTarget &target, sf::RenderStates states, sf::Vector2f center, float radius, float outline_thickness,
                          sf::Color color, int points_per_segment ) const
{
  if ( !visible ) return;

  // Generate vertices if needed (using cached vertices)
  if ( vertices_dirty || cached_radius != radius || cached_outline_thickness != outline_thickness || cached_position != center ||
       cached_color != color )
  {
    generateVertices( center, radius, outline_thickness, color, points_per_segment );
  }

  // Draw the cached vertices
  if ( !cached_vertices.empty() )
  {
    sf::VertexArray vertex_array( sf::PrimitiveType::Triangles );
    for ( const auto &vertex : cached_vertices )
    {
      vertex_array.append( vertex );
    }
    target.draw( vertex_array, states );
  }
}

void CircleSegment::generateVertices( sf::Vector2f center, float radius, float outline_thickness, sf::Color color, int points_per_segment ) const
{
  cached_vertices.clear();

  float angle_range = end_angle - start_angle;
  if ( angle_range <= 0 ) return;

  int num_points = std::max( 2, static_cast<int>( points_per_segment * angle_range / ( 2 * std::numbers::pi ) ) );

  float inner_radius = radius - outline_thickness / 2.0f;
  float outer_radius = radius + outline_thickness / 2.0f;

  for ( int i = 0; i < num_points - 1; ++i )
  {
    float t1 = static_cast<float>( i ) / ( num_points - 1 );
    float t2 = static_cast<float>( i + 1 ) / ( num_points - 1 );

    float angle1 = start_angle + t1 * angle_range;
    float angle2 = start_angle + t2 * angle_range;

    // Calculate inner and outer points for both angles
    sf::Vector2f inner1 = center + sf::Vector2f( std::cos( angle1 ) * inner_radius, std::sin( angle1 ) * inner_radius );
    sf::Vector2f outer1 = center + sf::Vector2f( std::cos( angle1 ) * outer_radius, std::sin( angle1 ) * outer_radius );
    sf::Vector2f inner2 = center + sf::Vector2f( std::cos( angle2 ) * inner_radius, std::sin( angle2 ) * inner_radius );
    sf::Vector2f outer2 = center + sf::Vector2f( std::cos( angle2 ) * outer_radius, std::sin( angle2 ) * outer_radius );

    // Create two triangles to form a quad segment
    // Triangle 1: inner1, outer1, inner2
    cached_vertices.emplace_back( inner1, color );
    cached_vertices.emplace_back( outer1, color );
    cached_vertices.emplace_back( inner2, color );

    // Triangle 2: outer1, outer2, inner2
    cached_vertices.emplace_back( outer1, color );
    cached_vertices.emplace_back( outer2, color );
    cached_vertices.emplace_back( inner2, color );
  }

  // Update cache state
  vertices_dirty = false;
  cached_radius = radius;
  cached_outline_thickness = outline_thickness;
  cached_position = center;
  cached_color = color;
}

} // namespace ProceduralMaze::Sprites