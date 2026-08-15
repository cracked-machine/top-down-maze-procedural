#ifndef SRC_COMPONENTS_RUIN_STAIRSLOWERMULTIBLOCK_HPP__
#define SRC_COMPONENTS_RUIN_STAIRSLOWERMULTIBLOCK_HPP__

#include <SFML/Graphics/RectangleShape.hpp>

#include <Components/Obstacle.hpp>

namespace Game::Cmp::Ruin
{

// RuinMultiBlock component represents a multi-block sprite defined geometrically as a rectangle
// It can be used to find its RuinSegment parts and track overall activation state
class StairsLowerMultiBlock : public sf::FloatRect
{
public:
  StairsLowerMultiBlock( const sf::Vector2f &position, const sf::Vector2f &size )
      : sf::FloatRect( position, size )
  {
  }
};

} // namespace Game::Cmp::Ruin

#endif // SRC_COMPONENTS_RUIN_STAIRSLOWERMULTIBLOCK_HPP__
