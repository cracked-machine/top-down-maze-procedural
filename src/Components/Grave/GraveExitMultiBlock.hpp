#ifndef SRC_COMPONENTS_GRAVE_GRAVEEXITMULTIBLOCK_HPP__
#define SRC_COMPONENTS_GRAVE_GRAVEEXITMULTIBLOCK_HPP__

#include <SFML/Graphics/RectangleShape.hpp>

namespace Game::Cmp
{

// GraveMultiBlock component represents a multi-block sprite defined geometrically as a rectangle
// It can be used to find its GraveSegment parts and track overall activation state
class GraveExitMultiBlock : public sf::FloatRect
{
public:
  GraveExitMultiBlock( const sf::Vector2f &position, const sf::Vector2f &size )
      : sf::FloatRect( position, size )
  {
  }
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_GRAVE_GRAVEEXITMULTIBLOCK_HPP__
