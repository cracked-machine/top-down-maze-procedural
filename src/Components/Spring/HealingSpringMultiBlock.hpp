#ifndef SRC_COMPONENTS_SPRING_SACREDSPRINGMULTIBLOCK_HPP__
#define SRC_COMPONENTS_SPRING_SACREDSPRINGMULTIBLOCK_HPP__

#include <SFML/Graphics/RectangleShape.hpp>

#include <Components/Obstacle.hpp>

namespace Game::Cmp
{

// WellMultiBlock component represents a multi-block sprite defined geometrically as a rectangle
// It can be used to find its WellSegment parts and track overall activation state
// NOTE: This is used for both exterior well buildings and the interior well itself
class HealingSpringMultiBlock : public sf::FloatRect
{
public:
  HealingSpringMultiBlock( const sf::Vector2f &position, const sf::Vector2f &size )
      : sf::FloatRect( position, size )
  {
  }
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_SPRING_SACREDSPRINGMULTIBLOCK_HPP__
