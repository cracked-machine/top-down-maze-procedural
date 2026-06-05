#include <Constants.hpp>
#include <Sprites/Shockwave.hpp>
#include <cmath>
#include <numbers>

namespace Game::Sprites
{

Shockwave::Shockwave( sf::Vector2f pos, int points_per_segment )
    : m_position( pos ),
      m_radius( 0 ),
      m_outline_color( sf::Color{ 254, 254, 254, 16 } ),
      m_outline_thickness( Constants::kGridSizePxF.x ),
      m_points_per_segment( points_per_segment )
{
  // Initialize with full circle
  m_segments.emplace_back( 0.0f, 2 * std::numbers::pi, true );
}

bool Shockwave::intersects_with_point( sf::Vector2f point ) const
{
  double distance = std::sqrt( std::pow( point.x - m_position.x, 2 ) + std::pow( point.y - m_position.y, 2 ) );
  return std::abs( distance - m_radius ) <= m_outline_thickness;
}

bool Shockwave::intersects_with_rect( const sf::FloatRect &rect ) const
{
  // Check corners and edges of rectangle against circle outline
  sf::Vector2f corners[4] = { { rect.position.x, rect.position.y },                             // top left
                              { rect.position.x + rect.size.x, rect.position.y },               // top right
                              { rect.position.x + rect.size.x, rect.position.y + rect.size.y }, // bottom right
                              { rect.position.x, rect.position.y + rect.size.y } };             // bottom left

  // Check corners
  for ( const auto &corner : corners )
  {
    if ( intersects_with_point( corner ) ) return true;
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

Shockwave::CircleSegments Shockwave::get_visible_segments() const
{
  CircleSegments visible_segments;
  for ( const auto &segment : m_segments )
  {
    if ( segment.is_visible() ) { visible_segments.push_back( segment ); }
  }
  return visible_segments;
}

void Shockwave::invalidate_all_segments()
{
  for ( auto &segment : m_segments )
  {
    segment.invalidate_cache();
  }
}

} // namespace Game::Sprites