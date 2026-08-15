#ifndef SRC_COMPONENTS_PLAYER_LASTGRAVEYARDPOSITION_HPP__
#define SRC_COMPONENTS_PLAYER_LASTGRAVEYARDPOSITION_HPP__

#include <Components/Position.hpp>

namespace Game::Cmp::Player
{

class LastGraveyardPosition : public Cmp::Position
{
public:
  LastGraveyardPosition( const sf::Vector2f &pos, const sf::Vector2f &size )
      : Cmp::Position( pos, size )
  {
  }
};

} // namespace Game::Cmp::Player

#endif // SRC_COMPONENTS_PLAYER_LASTGRAVEYARDPOSITION_HPP__
