#ifndef SRC_SPRITES_SHOCKWAVESPRITE_
#define SRC_SPRITES_SHOCKWAVESPRITE_

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <Sprites/CircleSegment.hpp>
#include <Utils/Constants.hpp>

namespace ProceduralMaze::Sprites
{

class Shockwave
{
public:
  //! @brief Circle segments contained in this Shockwave
  using CircleSegments = std::vector<CircleSegment>;

  //! @brief Construct a new Shockwave Sprite object
  //! @param pos
  //! @param points_per_segment
  Shockwave( sf::Vector2f pos, int points_per_segment );

  //! @brief Get the Points Per Segment object
  //! @return int
  int getPointsPerSegment() const { return m_points_per_segment; }

  //! @brief Set the Position object
  //! @param pos
  void setPosition( sf::Vector2f pos )
  {
    m_position = pos;
    invalidateAllSegments();
  }

  //! @brief Get the Position object
  //! @return sf::Vector2f
  sf::Vector2f getPosition() const { return m_position; }

  //! @brief Set the Radius object
  //! @param radius
  void setRadius( float radius )
  {
    m_radius = radius;
    invalidateAllSegments();
  }

  //! @brief Get the Radius object
  //! @return float
  float getRadius() const { return m_radius; }

  //! @brief Set the Outline Color object
  //! @param color
  void setOutlineColor( sf::Color color )
  {
    m_outline_color = color;
    invalidateAllSegments();
  }

  //! @brief Get the Outline Color object
  //! @return sf::Color
  sf::Color getOutlineColor() const { return m_outline_color; }

  //! @brief Set the Outline Thickness object
  //! @param thickness
  void setOutlineThickness( float thickness )
  {
    m_outline_thickness = thickness;
    invalidateAllSegments();
  }

  //! @brief Get the Segments object
  //! @return const CircleSegments&
  const CircleSegments &getSegments() const { return m_segments; }

  //! @brief Set the Segments object
  //! @param segments
  void setSegments( CircleSegments &&segments )
  {
    m_segments = std::move( segments );
    invalidateAllSegments();
  }

  //! @brief Get the Outline Thickness object
  //! @return float
  float getOutlineThickness() const { return m_outline_thickness; }

  //! @brief Check if a point intersects with the circle outline
  //! @param point
  //! @return true
  //! @return false
  bool intersectsWithPoint( sf::Vector2f point ) const;

  //! @brief Check if a rectangle intersects with any part of the circle outline
  //! @param rect
  //! @return true
  //! @return false
  bool intersectsWithRect( const sf::FloatRect &rect ) const;

  //! @brief Get a list of all visible segments (for debugging or other purposes)
  //! @return CircleSegments
  CircleSegments getVisibleSegments() const;

private:
  sf::Vector2f m_position;
  float m_radius;
  sf::Color m_outline_color;
  float m_outline_thickness;
  CircleSegments m_segments;
  int m_points_per_segment;

  //! @brief Invalidate all segment caches
  void invalidateAllSegments();
};

} // namespace ProceduralMaze::Sprites

#endif // SRC_SPRITES_SHOCKWAVESPRITE_