#ifndef SRC_COMPONENTS_SELECTEDPOSITION_HPP__
#define SRC_COMPONENTS_SELECTEDPOSITION_HPP__

#include <SFML/System/Vector2.hpp>

namespace Game::Cmp
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

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_SELECTEDPOSITION_HPP__
