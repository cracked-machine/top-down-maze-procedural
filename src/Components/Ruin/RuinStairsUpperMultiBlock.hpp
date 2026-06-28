#ifndef SRC_COMPONENTS_RUIN_RUINSTAIRSUPPERMULTIBLOCK_HPP__
#define SRC_COMPONENTS_RUIN_RUINSTAIRSUPPERMULTIBLOCK_HPP__

#include <SFML/Graphics/RectangleShape.hpp>

#include <Components/Obstacle.hpp>

namespace Game::Cmp
{

// RuinMultiBlock component represents a multi-block sprite defined geometrically as a rectangle
// It can be used to find its RuinSegment parts and track overall activation state
class RuinStairsUpperMultiBlock : public sf::FloatRect
{
public:
  RuinStairsUpperMultiBlock( const sf::Vector2f &position, const sf::Vector2f &size )
      : sf::FloatRect( position, size )
  {
  }
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_RUIN_RUINSTAIRSUPPERMULTIBLOCK_HPP__
