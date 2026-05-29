#ifndef SRC_COMPONENTS_RUINSTAIRSGATEMULTIBLOCK_HPP_
#define SRC_COMPONENTS_RUINSTAIRSGATEMULTIBLOCK_HPP_

#include <SFML/Graphics/RectangleShape.hpp>

#include <Components/Obstacle.hpp>

namespace Game::Cmp
{

//! @brief This gate blocks the player from accessing the stairs until the ruinlowerscene puzzle is completed.
class RuinStairsGateMultiBlock : public sf::FloatRect
{
public:
  RuinStairsGateMultiBlock( const sf::Vector2f &position, const sf::Vector2f &size )
      : sf::FloatRect( position, size )
  {
  }
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_RUINSTAIRSGATEMULTIBLOCK_HPP_