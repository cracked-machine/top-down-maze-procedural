#ifndef SRC_COMPONENTS_DEATHPOSITION_HPP__
#define SRC_COMPONENTS_DEATHPOSITION_HPP__

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace Game::Cmp
{

class DeathPosition : public sf::FloatRect
{
public:
  DeathPosition( const sf::Vector2f &pos, const sf::Vector2f &size )
      : sf::FloatRect( pos, size )
  {
  }
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_DEATHPOSITION_HPP__
