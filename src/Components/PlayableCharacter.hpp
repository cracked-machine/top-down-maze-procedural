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
  // 0 is no bombs, -1 is infinite bombs
  int bomb_inventory{};
  // blast radius for bomb explosions
  int blast_radius{};

  // prevent bomb spamming
  sf::Clock m_bombdeploycooldowntimer;
  // prevent bomb spamming
  const sf::Time m_bombdeploydelay{ sf::seconds( 0.5 ) };
  // prevent placing more than one bomb at a time

  bool has_active_bomb{ false };

  bool underwater{ false };

  sf::Clock m_damage_cooldown_timer;

  PlayableCharacter( int bomb_inv, int blast_rad )
      : bomb_inventory( bomb_inv ),
        blast_radius( blast_rad )
  {
  }

  // Custom copy constructor to handle sf::Clock
  PlayableCharacter( const PlayableCharacter &other )
      : bomb_inventory( other.bomb_inventory ),
        blast_radius( other.blast_radius ),
        m_bombdeploycooldowntimer(), // Reset clocks on copy
        m_bombdeploydelay( other.m_bombdeploydelay ),
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
      bomb_inventory = other.bomb_inventory;
      blast_radius = other.blast_radius;
      // Reset clocks instead of copying
      m_bombdeploycooldowntimer.restart();
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