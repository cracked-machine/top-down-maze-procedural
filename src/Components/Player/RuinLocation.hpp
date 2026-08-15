#ifndef SRC_COMPONENTS_PLAYER_RUINLOCATION_HPP__
#define SRC_COMPONENTS_PLAYER_RUINLOCATION_HPP__

namespace Game::Cmp::Player
{

class RuinLocation
{
public:
  enum class Floor { NONE, LOWER, UPPER };
  RuinLocation( Floor floor )
      : m_floor( floor )
  {
  }
  Floor m_floor;
};

} // namespace Game::Cmp::Player

#endif // SRC_COMPONENTS_PLAYER_RUINLOCATION_HPP__
