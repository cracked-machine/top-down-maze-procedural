#ifndef SRC_COMPONENTS_PLANTMULTIBLOCK_HPP__
#define SRC_COMPONENTS_PLANTMULTIBLOCK_HPP__

#include <SFML/Graphics/Rect.hpp>
namespace Game::Cmp
{

//! @brief Represents a decorative plant obstacle as a multi-block sprite defined geometrically as a
//! rectangle.
//! @details Can be used to find its PlantSegment parts and track overall activation state.
class PlantMultiBlock : public sf::FloatRect
{
public:
  //! @brief Construct a new PlantMultiBlock object.
  //! @param position Top-left world position of the multi-block bounds.
  //! @param size Width/height of the multi-block bounds.
  PlantMultiBlock( const sf::Vector2f &position, const sf::Vector2f &size )
      : sf::FloatRect( position, size )
  {
  }
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_PLANTMULTIBLOCK_HPP__
