#ifndef __COMPONENTS_POSITION_HPP__
#define __COMPONENTS_POSITION_HPP__

#include <Base.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Cmp {

// Coordinate
class Position : public sf::FloatRect
{
public:
  Position( const sf::Vector2f &pos, const sf::Vector2f &size )
      : sf::FloatRect( pos, size )
  {
  }
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_POSITION_HPP__