#ifndef SRC_CMPS_PLAYER_PLAYERRUINFLOOR_HPP_
#define SRC_CMPS_PLAYER_PLAYERRUINFLOOR_HPP_

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

#endif // SRC_CMPS_PLAYER_PLAYERRUINFLOOR_HPP_