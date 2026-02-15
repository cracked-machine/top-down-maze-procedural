#ifndef SRC_CMPS_PLAYER_PLAYERCHARACTER_HPP_
#define SRC_CMPS_PLAYER_PLAYERCHARACTER_HPP_

#include <SFML/System/Clock.hpp>

namespace ProceduralMaze::Cmp
{

// See PlayerSystem.hpp for initilization values
class PlayerCharacter
{
public:
  bool has_active_bomb{ false };

  bool underwater{ false };

  sf::Clock m_damage_cooldown_timer;

  // Custom copy constructor to handle sf::Clock
  PlayerCharacter( const PlayerCharacter &other )
      : has_active_bomb( other.has_active_bomb ),
        underwater( other.underwater ),
        m_damage_cooldown_timer() // Reset clocks on copy
  {
  }

  // Custom copy assignment operator
  PlayerCharacter &operator=( const PlayerCharacter &other )
  {
    if ( this != &other )
    {

      has_active_bomb = other.has_active_bomb;
      underwater = other.underwater;
      m_damage_cooldown_timer.restart();
    }
    return *this;
  }

  // Default constructor
  PlayerCharacter() = default;
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_PLAYER_PLAYERCHARACTER_HPP_