#ifndef SRC_COMPONENTS_HOLYWELLMULTIBLOCK_HPP__
#define SRC_COMPONENTS_HOLYWELLMULTIBLOCK_HPP__

#include <SFML/Graphics/RectangleShape.hpp>

#include <Components/Obstacle.hpp>

namespace ProceduralMaze::Cmp
{

// HolyWellMultiBlock component represents a multi-block sprite defined geometrically as a rectangle
// It can be used to find its HolyWellSegment parts and track overall activation state
// NOTE: This is used for both exterior well buildings and the interior well itself
class HolyWellMultiBlock : public sf::FloatRect
{
public:
  HolyWellMultiBlock( const sf::Vector2f &position, const sf::Vector2f &size )
      : sf::FloatRect( position, size )
  {
  }
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_HOLYWELLMULTIBLOCK_HPP__