#ifndef SRC_COMPONENTS_PLAYER_PLAYERCHARACTER_HPP__
#define SRC_COMPONENTS_PLAYER_PLAYERCHARACTER_HPP__

#include <SFML/System/Clock.hpp>

namespace Game::Cmp
{

// See PlayerSystem.hpp for initilization values
class PlayerCharacter
{
public:
  bool has_active_bomb{ false };

  bool underwater{ false };

  // When set, the next damage check bypasses the cooldown timer entirely so the hit always lands,
  // regardless of how recently the player was last damaged. Cleared once that hit is applied.
  bool skip_damage_cooldown_once{ false };

  sf::Clock m_damage_cooldown_timer;

  // Custom copy constructor to handle sf::Clock
  PlayerCharacter( const PlayerCharacter &other )
      : has_active_bomb( other.has_active_bomb ),
        underwater( other.underwater ),
        skip_damage_cooldown_once( other.skip_damage_cooldown_once ),
        m_damage_cooldown_timer(),
        m_global_bomb_flash_clk() // Reset clocks on copy
  {
  }

  // Custom copy assignment operator
  PlayerCharacter &operator=( const PlayerCharacter &other )
  {
    if ( this != &other )
    {

      has_active_bomb = other.has_active_bomb;
      underwater = other.underwater;
      skip_damage_cooldown_once = other.skip_damage_cooldown_once;
      m_damage_cooldown_timer.restart();
      m_global_bomb_flash_clk.reset();
    }
    return *this;
  }

  // Default constructor
  PlayerCharacter() = default;

  sf::Clock m_global_bomb_flash_clk;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_PLAYER_PLAYERCHARACTER_HPP__
