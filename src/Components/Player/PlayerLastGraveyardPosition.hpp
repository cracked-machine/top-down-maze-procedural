#ifndef SRC_COMPONENTS_PLAYER_PLAYERLASTGRAVEYARDPOSITION_HPP__
#define SRC_COMPONENTS_PLAYER_PLAYERLASTGRAVEYARDPOSITION_HPP__

#include <Components/Position.hpp>

namespace Game::Cmp
{

class PlayerLastGraveyardPosition : public Cmp::Position
{
public:
  PlayerLastGraveyardPosition( const sf::Vector2f &pos, const sf::Vector2f &size )
      : Cmp::Position( pos, size )
  {
  }
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_PLAYER_PLAYERLASTGRAVEYARDPOSITION_HPP__
