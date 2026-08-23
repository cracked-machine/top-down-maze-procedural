#ifndef SRC_COMPONENTS_DEATHPOSITION_HPP__
#define SRC_COMPONENTS_DEATHPOSITION_HPP__

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace Game::Cmp
{

//! @brief Component storing the world-space rectangle where an entity died.
class DeathPosition : public sf::FloatRect
{
public:
  //! @brief Construct from a position and size.
  //! @param pos World-space top-left position.
  //! @param size Width/height of the rectangle.
  DeathPosition( const sf::Vector2f &pos, const sf::Vector2f &size )
      : sf::FloatRect( pos, size )
  {
  }
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_DEATHPOSITION_HPP__
