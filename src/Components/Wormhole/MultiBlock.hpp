#ifndef SRC_COMPONENTS_WORMHOLE_MULTIBLOCK_HPP__
#define SRC_COMPONENTS_WORMHOLE_MULTIBLOCK_HPP__

#include <SFML/Graphics/RectangleShape.hpp>

#include <Components/Obstacle.hpp>

namespace Game::Cmp::Wormhole
{

// MultiBlock component represents a multi-block sprite defined geometrically as a rectangle
// It also tracks activation of its constituent sprites and overall activation state
class MultiBlock : public sf::FloatRect
{
public:
  MultiBlock( const sf::Vector2f &position, const sf::Vector2f &size )
      : sf::FloatRect( position, size )
  {
  }
};

} // namespace Game::Cmp::Wormhole

#endif // SRC_COMPONENTS_WORMHOLE_MULTIBLOCK_HPP__
