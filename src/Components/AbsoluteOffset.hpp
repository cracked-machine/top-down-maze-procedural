#ifndef INC_COMPONENTS_ABSOLUTEOFFSET_HPP
#define INC_COMPONENTS_ABSOLUTEOFFSET_HPP

#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Cmp
{

class AbsoluteOffset
{
public:
  AbsoluteOffset( const sf::Vector2f &offset = sf::Vector2f( 0.f, 0.f ) )
      : m_offset( offset )
  {
  }

  AbsoluteOffset( float x, float y )
      : m_offset( { x, y } )
  {
  }

  const sf::Vector2f &getOffset() const { return m_offset; }
  void setOffset( const sf::Vector2f &offset ) { m_offset = offset; }
  void setOffsetX( float x ) { m_offset.x = x; }
  void setOffsetY( float y ) { m_offset.y = y; }

private:
  sf::Vector2f m_offset;
};

} // namespace ProceduralMaze::Cmp

#endif // INC_COMPONENTS_ABSOLUTEOFFSET_HPP