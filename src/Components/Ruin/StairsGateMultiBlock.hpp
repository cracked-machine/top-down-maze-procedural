#ifndef SRC_COMPONENTS_RUIN_STAIRSGATEMULTIBLOCK_HPP__
#define SRC_COMPONENTS_RUIN_STAIRSGATEMULTIBLOCK_HPP__

#include <SFML/Graphics/RectangleShape.hpp>

#include <Components/Obstacle.hpp>

namespace Game::Cmp::Ruin
{

//! @brief This gate blocks the player from accessing the stairs until the ruinlowerscene puzzle is completed.
class StairsGateMultiBlock : public sf::FloatRect
{
public:
  StairsGateMultiBlock( const sf::Vector2f &position, const sf::Vector2f &size )
      : sf::FloatRect( position, size )
  {
  }
};

} // namespace Game::Cmp::Ruin

#endif // SRC_COMPONENTS_RUIN_STAIRSGATEMULTIBLOCK_HPP__
