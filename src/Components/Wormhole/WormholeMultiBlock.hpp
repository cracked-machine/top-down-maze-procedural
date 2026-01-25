#ifndef SRC_COMPONENTS_WORMHOLEMULTIBLOCK_HPP__
#define SRC_COMPONENTS_WORMHOLEMULTIBLOCK_HPP__

#include <SFML/Graphics/RectangleShape.hpp>

#include <Components/Obstacle.hpp>

namespace ProceduralMaze::Cmp
{

// WormholeMultiBlock component represents a multi-block sprite defined geometrically as a rectangle
// It also tracks activation of its constituent sprites and overall activation state
class WormholeMultiBlock : public sf::FloatRect
{
public:
  WormholeMultiBlock( const sf::Vector2f &position, const sf::Vector2f &size )
      : sf::FloatRect( position, size )
  {
  }
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_GRAVEMULTIBLOCK_HPP__