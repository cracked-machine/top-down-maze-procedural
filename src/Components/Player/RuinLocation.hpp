#ifndef SRC_COMPONENTS_PLAYER_RUINLOCATION_HPP__
#define SRC_COMPONENTS_PLAYER_RUINLOCATION_HPP__

namespace Game::Cmp::Player
{

//! @brief Tracks which floor of a ruin the player is currently located on.
class RuinLocation
{
public:
  //! @brief Which floor of a ruin the player occupies.
  enum class Floor {
    //! @brief Player is not inside a ruin.
    NONE,
    //! @brief Player is on the lower floor of a ruin.
    LOWER,
    //! @brief Player is on the upper floor of a ruin.
    UPPER
  };
  //! @brief Construct with an initial floor.
  //! @param floor initial floor value
  RuinLocation( Floor floor )
      : m_floor( floor )
  {
  }
  //! @brief The floor the player currently occupies.
  Floor m_floor;
};

} // namespace Game::Cmp::Player

#endif // SRC_COMPONENTS_PLAYER_RUINLOCATION_HPP__
