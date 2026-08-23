#ifndef SRC_COMPONENTS_GRAVE_EXITMULTIBLOCK_HPP__
#define SRC_COMPONENTS_GRAVE_EXITMULTIBLOCK_HPP__

#include <SFML/Graphics/RectangleShape.hpp>

namespace Game::Cmp::Grave
{

//! @brief Represents the level exit structure as a multi-block sprite defined geometrically as a rectangle.
//! @details Can be used to find its ExitSegment parts and track overall activation state.
class ExitMultiBlock : public sf::FloatRect
{
public:
  //! @brief Construct a new ExitMultiBlock object.
  //! @param position Top-left world position of the multi-block bounds.
  //! @param size Width/height of the multi-block bounds.
  ExitMultiBlock( const sf::Vector2f &position, const sf::Vector2f &size )
      : sf::FloatRect( position, size )
  {
  }
};

} // namespace Game::Cmp::Grave

#endif // SRC_COMPONENTS_GRAVE_EXITMULTIBLOCK_HPP__
