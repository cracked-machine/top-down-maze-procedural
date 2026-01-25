#ifndef __COMPONENTS_SELECTED_POSITION_HPP__
#define __COMPONENTS_SELECTED_POSITION_HPP__

#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Cmp
{

//! @brief Used for marking an entity as selected by mouse pointer, e.g. when digging, etc...
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