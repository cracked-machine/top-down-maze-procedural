#ifndef SRC_COMPONENTS_PLAYER_LASTGRAVEYARDPOSITION_HPP__
#define SRC_COMPONENTS_PLAYER_LASTGRAVEYARDPOSITION_HPP__

#include <Components/Position.hpp>

namespace Game::Cmp::Player
{

//! @brief Records the player's most recent position within a graveyard scene, used as a respawn/return point.
class LastGraveyardPosition : public Cmp::Position
{
public:
  //! @brief Construct from a position and size.
  //! @param pos world-space position
  //! @param size bounding size
  LastGraveyardPosition( const sf::Vector2f &pos, const sf::Vector2f &size )
      : Cmp::Position( pos, size )
  {
  }
};

} // namespace Game::Cmp::Player

#endif // SRC_COMPONENTS_PLAYER_LASTGRAVEYARDPOSITION_HPP__
