#ifndef SRC_CMPS_NPCSHOCKWAVE_
#define SRC_CMPS_NPCSHOCKWAVE_

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
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

    // Vertex caching members
    mutable std::vector<sf::Vertex> cached_vertices;
    mutable bool vertices_dirty = true;
    mutable float cached_radius = -1.0f;
    mutable float cached_outline_thickness = -1.0f;
    mutable sf::Vector2f cached_position = { -1.0f, -1.0f };
    mutable sf::Color cached_color = sf::Color::Transparent;

    CircleSegment( float start, float end, bool vis = true )
        : start_angle( start ),
          end_angle( end ),
          visible( vis )
    {
    }

    sf::FloatRect getBounds( sf::Vector2f center, float radius, float outline_thickness ) const
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

    void draw( sf::RenderTarget &target, sf::RenderStates states, sf::Vector2f center, float radius, float outline_thickness, sf::Color color,
               int points_per_segment ) const
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

    void invalidateCache() const { vertices_dirty = true; }

  private:
    sf::Vector2f start_point;
    sf::Vector2f end_point;

    void generateVertices( sf::Vector2f center, float radius, float outline_thickness, sf::Color color, int points_per_segment ) const
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
  };

private:
  sf::Vector2f m_position;
  float m_radius;
  sf::Color m_outline_color;
  float m_outline_thickness;
  std::vector<CircleSegment> m_segments;
  mutable sf::VertexArray m_vertices;
  int m_points_per_segment;

  void invalidateAllSegments()
  {
    for ( auto &segment : m_segments )
    {
      segment.invalidateCache();
    }
  }

public:
  NpcShockwave( sf::Vector2f pos, int points_per_segment )
      : m_position( pos ),
        m_radius( Constants::kGridSquareSizePixelsF.x ),
        m_outline_color( sf::Color::Red ),
        m_outline_thickness( 2.f ),
        m_vertices( sf::PrimitiveType::Lines ),
        m_points_per_segment( points_per_segment )
  {
    // Initialize with full circle
    m_segments.emplace_back( 0.0f, 2 * std::numbers::pi, true );
  }

  int getPointsPerSegment() { return m_points_per_segment; }
  void setPosition( sf::Vector2f pos )
  {
    m_position = pos;
    invalidateAllSegments();
  }

  sf::Vector2f getPosition() const { return m_position; }

  void setRadius( float radius )
  {
    m_radius = radius;
    invalidateAllSegments();
  }

  float getRadius() const { return m_radius; }

  void setOutlineColor( sf::Color color )
  {
    m_outline_color = color;
    invalidateAllSegments();
  }

  sf::Color getOutlineColor() { return m_outline_color; }

  void setOutlineThickness( float thickness )
  {
    m_outline_thickness = thickness;
    invalidateAllSegments();
  }

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

      float angle_range = segment.end_angle - segment.start_angle;

      for ( int i = 0; i < m_points_per_segment; ++i )
      {
        float t = static_cast<float>( i ) / ( m_points_per_segment - 1 );
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
  virtual void draw( sf::RenderTarget &target, sf::RenderStates states ) const override { target.draw( m_vertices, states ); }
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_NPCSHOCKWAVE_