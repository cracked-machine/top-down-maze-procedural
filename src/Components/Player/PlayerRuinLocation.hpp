#ifndef SRC_COMPONENTS_PLAYER_PLAYERRUINLOCATION_HPP__
#define SRC_COMPONENTS_PLAYER_PLAYERRUINLOCATION_HPP__

namespace Game::Cmp
{

class PlayerRuinLocation
{
public:
  enum class Floor { NONE, LOWER, UPPER };
  PlayerRuinLocation( Floor floor )
      : m_floor( floor )
  {
  }
  Floor m_floor;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_PLAYER_PLAYERRUINLOCATION_HPP__
