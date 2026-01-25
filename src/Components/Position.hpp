#ifndef __COMPONENTS_POSITION_HPP__
#define __COMPONENTS_POSITION_HPP__

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Cmp
{

//! @brief Component used to contain a grid position (in pixels). Also contains the size of the grid square (usually 16x16px)
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