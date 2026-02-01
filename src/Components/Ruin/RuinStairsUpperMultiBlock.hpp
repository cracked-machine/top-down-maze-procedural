#ifndef SRC_COMPONENTS_RUINSTAIRSUPPERMULTIBLOCK_HPP_
#define SRC_COMPONENTS_RUINSTAIRSUPPERMULTIBLOCK_HPP_

#include <SFML/Graphics/RectangleShape.hpp>

#include <Components/Obstacle.hpp>

namespace ProceduralMaze::Cmp
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

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_RUINSTAIRSUPPERMULTIBLOCK_HPP_