#ifndef SRC_COMPONENTS_PLAYER_PLAYERLASTGRAVEYARDPOSITION_HPP_
#define SRC_COMPONENTS_PLAYER_PLAYERLASTGRAVEYARDPOSITION_HPP_

#include <Components/Position.hpp>

namespace ProceduralMaze::Cmp
{

class PlayerLastGraveyardPosition : public Cmp::Position
{
public:
  PlayerLastGraveyardPosition( const sf::Vector2f &pos, const sf::Vector2f &size )
      : Cmp::Position( pos, size )
  {
  }
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_PLAYER_PLAYERLASTGRAVEYARDPOSITION_HPP_