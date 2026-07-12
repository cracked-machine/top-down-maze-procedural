#ifndef SRC_COMPONENTS_PLANTMULTIBLOCK_HPP__
#define SRC_COMPONENTS_PLANTMULTIBLOCK_HPP__

#include <SFML/Graphics/Rect.hpp>
namespace Game::Cmp
{

//! @brief Mark the entity as a plant obstacle
class PlantMultiBlock : public sf::FloatRect
{
public:
  PlantMultiBlock( const sf::Vector2f &position, const sf::Vector2f &size )
      : sf::FloatRect( position, size )
  {
  }
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_PLANTMULTIBLOCK_HPP__
