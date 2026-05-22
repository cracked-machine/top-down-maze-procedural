#ifndef __CMP_NPC_DEATH_POSITION_HPP__
#define __CMP_NPC_DEATH_POSITION_HPP__

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

#endif // __CMP_NPC_DEATH_POSITION_HPP__