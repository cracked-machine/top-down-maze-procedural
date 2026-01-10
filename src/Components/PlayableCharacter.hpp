#ifndef __COMPONENTS_PLAYABLECHARACTER_HPP__
#define __COMPONENTS_PLAYABLECHARACTER_HPP__

#include <spdlog/spdlog.h>

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>

namespace ProceduralMaze::Cmp
{

// See PlayerSystem.hpp for initilization values
class PlayableCharacter
{
public:
  // blast radius for bomb explosions
  int blast_radius{};

  bool has_active_bomb{ false };

  bool underwater{ false };

  sf::Clock m_damage_cooldown_timer;

  PlayableCharacter( int blast_rad )
      : blast_radius( blast_rad )
  {
  }

  // Custom copy constructor to handle sf::Clock
  PlayableCharacter( const PlayableCharacter &other )
      : blast_radius( other.blast_radius ),
        has_active_bomb( other.has_active_bomb ),
        underwater( other.underwater ),
        m_damage_cooldown_timer() // Reset clocks on copy
  {
  }

  // Custom copy assignment operator
  PlayableCharacter &operator=( const PlayableCharacter &other )
  {
    if ( this != &other )
    {
      blast_radius = other.blast_radius;
      has_active_bomb = other.has_active_bomb;
      underwater = other.underwater;
      m_damage_cooldown_timer.restart();
    }
    return *this;
  }

  // Default constructor
  PlayableCharacter() = default;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_PLAYABLECHARACTER_HPP__