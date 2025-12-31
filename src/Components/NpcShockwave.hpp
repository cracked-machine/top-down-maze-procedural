#ifndef SRC_CMPS_NPCSHOCKWAVE_
#define SRC_CMPS_NPCSHOCKWAVE_

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <Utils/Utils.hpp>
#include <numbers>
#include <vector>

namespace ProceduralMaze::Cmp
{

class NpcShockwave : public sf::Drawable
{
public:
  struct CircleSegment
  {
    float start_angle;
    float end_angle;
    bool visible;

    CircleSegment( float start, float end, bool vis = true )
        : start_angle( start ),
          end_angle( end ),
          visible( vis )
    {
    }
  };

private:
  sf::Vector2f m_position;
  float m_radius;
  sf::Color m_outline_color;
  float m_outline_thickness;
  std::vector<CircleSegment> m_segments;
  mutable sf::VertexArray m_vertices;
  static constexpr int POINTS_PER_SEGMENT = 32;

  void updateVertices() const
  {
    m_vertices.clear();
    m_vertices.setPrimitiveType( sf::PrimitiveType::Triangles );

    for ( const auto &segment : m_segments )
    {
      if ( !segment.visible ) continue;

      float angle_range = segment.end_angle - segment.start_angle;
      if ( angle_range <= 0 ) continue;

      int num_points = std::max( 2, static_cast<int>( POINTS_PER_SEGMENT * angle_range / ( 2 * std::numbers::pi ) ) );

      float inner_radius = m_radius - m_outline_thickness / 2.0f;
      float outer_radius = m_radius + m_outline_thickness / 2.0f;

      for ( int i = 0; i < num_points - 1; ++i )
      {
        float t1 = static_cast<float>( i ) / ( num_points - 1 );
        float t2 = static_cast<float>( i + 1 ) / ( num_points - 1 );

        float angle1 = segment.start_angle + t1 * angle_range;
        float angle2 = segment.start_angle + t2 * angle_range;

        // Calculate inner and outer points for both angles
        sf::Vector2f inner1 = m_position + sf::Vector2f( std::cos( angle1 ) * inner_radius, std::sin( angle1 ) * inner_radius );
        sf::Vector2f outer1 = m_position + sf::Vector2f( std::cos( angle1 ) * outer_radius, std::sin( angle1 ) * outer_radius );
        sf::Vector2f inner2 = m_position + sf::Vector2f( std::cos( angle2 ) * inner_radius, std::sin( angle2 ) * inner_radius );
        sf::Vector2f outer2 = m_position + sf::Vector2f( std::cos( angle2 ) * outer_radius, std::sin( angle2 ) * outer_radius );

        // Create two triangles to form a quad segment
        // Triangle 1: inner1, outer1, inner2
        m_vertices.append( sf::Vertex( inner1, m_outline_color ) );
        m_vertices.append( sf::Vertex( outer1, m_outline_color ) );
        m_vertices.append( sf::Vertex( inner2, m_outline_color ) );

        // Triangle 2: outer1, outer2, inner2
        m_vertices.append( sf::Vertex( outer1, m_outline_color ) );
        m_vertices.append( sf::Vertex( outer2, m_outline_color ) );
        m_vertices.append( sf::Vertex( inner2, m_outline_color ) );
      }
    }
  }

public:
  NpcShockwave( sf::Vector2f pos )
      : m_position( pos ),
        m_radius( Constants::kGridSquareSizePixelsF.x ),
        m_outline_color( sf::Color::Red ),
        m_outline_thickness( 2.f ),
        m_vertices( sf::PrimitiveType::Lines )
  {
    // Initialize with full circle
    m_segments.emplace_back( 0.0f, 2 * std::numbers::pi, true );
  }

  void setPosition( sf::Vector2f pos ) { m_position = pos; }

  sf::Vector2f getPosition() const { return m_position; }

  void setRadius( float radius ) { m_radius = radius; }

  float getRadius() const { return m_radius; }

  void setOutlineColor( sf::Color color ) { m_outline_color = color; }

  void setOutlineThickness( float thickness ) { m_outline_thickness = thickness; }
  float getOutlineThickness() { return m_outline_thickness; }

  // Check if a point intersects with the circle outline
  bool intersectsWithPoint( sf::Vector2f point ) const
  {
    float distance = std::sqrt( std::pow( point.x - m_position.x, 2 ) + std::pow( point.y - m_position.y, 2 ) );
    return std::abs( distance - m_radius ) <= m_outline_thickness;
  }

  // Check if a rectangle intersects with any part of the circle outline
  bool intersectsWithRect( const sf::FloatRect &rect ) const
  {
    // Check corners and edges of rectangle against circle outline
    sf::Vector2f corners[4] = { { rect.position.x, rect.position.y },                             // top left
                                { rect.position.x + rect.size.x, rect.position.y },               // top right
                                { rect.position.x + rect.size.x, rect.position.y + rect.size.y }, // bottom right
                                { rect.position.x, rect.position.y + rect.size.y } };             // bottom left

    // Check corners
    for ( const auto &corner : corners )
    {
      if ( intersectsWithPoint( corner ) ) return true;
    }

    // Check if circle center is inside rectangle (circle intersects with rect)
    if ( rect.contains( m_position ) ) return true;

    // Check edges
    // Top and bottom edges
    if ( m_position.y >= rect.position.y - m_radius && m_position.y <= rect.position.y + rect.size.y + m_radius )
    {
      if ( m_position.x >= rect.position.x && m_position.x <= rect.position.x + rect.size.x ) return true;
    }

    // Left and right edges
    if ( m_position.x >= rect.position.x - m_radius && m_position.x <= rect.position.x + rect.size.x + m_radius )
    {
      if ( m_position.y >= rect.position.y && m_position.y <= rect.position.y + rect.size.y ) return true;
    }

    return false;
  }

  // Check if a rectangle intersects with any visible segment of the circle
  bool intersectsWithVisibleSegments( const sf::FloatRect &rect ) const
  {
    for ( const auto &segment : m_segments )
    {
      if ( not segment.visible ) continue;

      // Sample points along this visible segment
      constexpr int samples = 32;
      float angle_range = segment.end_angle - segment.start_angle;

      for ( int i = 0; i < samples; ++i )
      {
        float t = static_cast<float>( i ) / ( samples - 1 );
        float angle = segment.start_angle + t * angle_range;

        // Check both inner and outer radius points to account for thickness
        float inner_radius = m_radius - m_outline_thickness / 2.0f;
        float outer_radius = m_radius + m_outline_thickness / 2.0f;

        sf::Vector2f inner_point = m_position + sf::Vector2f( std::cos( angle ) * inner_radius, std::sin( angle ) * inner_radius );
        sf::Vector2f outer_point = m_position + sf::Vector2f( std::cos( angle ) * outer_radius, std::sin( angle ) * outer_radius );

        if ( rect.contains( inner_point ) || rect.contains( outer_point ) ) { return true; }
      }
    }
    return false;
  }

  // Get a list of all visible segments (for debugging or other purposes)
  std::vector<CircleSegment> getVisibleSegments() const
  {
    std::vector<CircleSegment> visible_segments;
    for ( const auto &segment : m_segments )
    {
      if ( segment.visible ) { visible_segments.push_back( segment ); }
    }
    return visible_segments;
  }

  // Check if a point intersects with any visible segment
  bool pointIntersectsVisibleSegments( sf::Vector2f point ) const
  {
    float distance = std::sqrt( std::pow( point.x - m_position.x, 2 ) + std::pow( point.y - m_position.y, 2 ) );

    // Check if point is at the right distance from center
    if ( std::abs( distance - m_radius ) > m_outline_thickness / 2.0f ) { return false; }

    // Calculate angle of the point relative to center
    float point_angle = std::atan2( point.y - m_position.y, point.x - m_position.x );
    if ( point_angle < 0 ) point_angle += 2 * std::numbers::pi;

    // Check if this angle falls within any visible segment
    for ( const auto &segment : m_segments )
    {
      if ( not segment.visible ) continue;

      float start_angle = segment.start_angle;
      float end_angle = segment.end_angle;

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

  // Remove segments that intersect with the given rectangle
  void removeIntersectingSegments( const sf::FloatRect &obstacle_rect )
  {
    std::vector<CircleSegment> new_segments;

    for ( const auto &segment : m_segments )
    {
      if ( not segment.visible ) continue;

      std::vector<CircleSegment> non_intersecting = splitSegmentByObstacle( segment, obstacle_rect );
      new_segments.insert( new_segments.end(), non_intersecting.begin(), non_intersecting.end() );
    }

    m_segments = std::move( new_segments );
  }

private:
  std::vector<CircleSegment> splitSegmentByObstacle( const CircleSegment &segment, const sf::FloatRect &obstacle_rect ) const
  {
    std::vector<CircleSegment> result;

    // Sample points along the segment to find intersections
    constexpr int samples = 64;
    std::vector<bool> intersections( samples, false );

    float angle_range = segment.end_angle - segment.start_angle;

    for ( int i = 0; i < samples; ++i )
    {
      float t = static_cast<float>( i ) / ( samples - 1 );
      float angle = segment.start_angle + t * angle_range;

      sf::Vector2f point = m_position + sf::Vector2f( std::cos( angle ) * m_radius, std::sin( angle ) * m_radius );

      // Only check if the circle outline point is inside the obstacle rectangle
      // Don't use intersectsWithPoint as it's too broad for this use case
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
        float start_angle = segment.start_angle + ( static_cast<float>( start_idx ) / ( samples - 1 ) ) * angle_range;
        float end_angle = segment.start_angle + ( static_cast<float>( i - 1 ) / ( samples - 1 ) ) * angle_range;

        if ( end_angle > start_angle ) { result.emplace_back( start_angle, end_angle, true ); }
        start_idx = -1;
      }
    }

    // Handle case where segment ends with non-intersecting part
    if ( start_idx != -1 )
    {
      float start_angle = segment.start_angle + ( static_cast<float>( start_idx ) / ( samples - 1 ) ) * angle_range;
      result.emplace_back( start_angle, segment.end_angle, true );
    }

    SPDLOG_DEBUG( "Split segment into {} parts", result.size() );

    // If no non-intersecting segments found, return empty vector
    // If the entire segment intersects, we want to remove it completely
    return result;
  }

public:
  virtual void draw( sf::RenderTarget &target, sf::RenderStates states ) const override
  {
    updateVertices();
    target.draw( m_vertices, states );
  }
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_NPCSHOCKWAVE_