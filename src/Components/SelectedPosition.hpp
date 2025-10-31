#ifndef __COMPONENTS_SELECTED_POSITION_HPP__
#define __COMPONENTS_SELECTED_POSITION_HPP__

#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Cmp {

// Coordinate
class SelectedPosition : public sf::Vector2f
{
public:
  SelectedPosition( const sf::Vector2f &pos )
      : sf::Vector2f( pos )
  {
  }
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_SELECTED_POSITION_HPP__