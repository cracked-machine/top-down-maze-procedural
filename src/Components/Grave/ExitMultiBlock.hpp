#ifndef SRC_COMPONENTS_GRAVE_EXITMULTIBLOCK_HPP__
#define SRC_COMPONENTS_GRAVE_EXITMULTIBLOCK_HPP__

#include <SFML/Graphics/RectangleShape.hpp>

namespace Game::Cmp::Grave
{

// MultiBlock component represents a multi-block sprite defined geometrically as a rectangle
// It can be used to find its Segment parts and track overall activation state
class ExitMultiBlock : public sf::FloatRect
{
public:
  ExitMultiBlock( const sf::Vector2f &position, const sf::Vector2f &size )
      : sf::FloatRect( position, size )
  {
  }
};

} // namespace Game::Cmp::Grave

#endif // SRC_COMPONENTS_GRAVE_EXITMULTIBLOCK_HPP__
