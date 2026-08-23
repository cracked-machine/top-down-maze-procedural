#ifndef SRC_COMPONENTS_PLAYER_CHARACTER_HPP__
#define SRC_COMPONENTS_PLAYER_CHARACTER_HPP__

#include <SFML/System/Clock.hpp>

namespace Game::Cmp::Player
{

//! @brief Core per-frame state for the player character (bomb/underwater/damage flags and timers).
//! @note See PlayerSystem.hpp for initialization values.
class Character
{
public:
  //! @brief Whether the player currently has a bomb armed/active.
  bool has_active_bomb{ false };

  //! @brief Whether the player is currently underwater.
  bool underwater{ false };

  //! @brief When set, the next damage check bypasses the cooldown timer entirely so the hit always lands,
  //!        regardless of how recently the player was last damaged. Cleared once that hit is applied.
  bool skip_damage_cooldown_once{ false };

  //! @brief Timer tracking time since the player was last damaged, used to gate the damage cooldown.
  sf::Clock m_damage_cooldown_timer;

  //! @brief Custom copy constructor to handle sf::Clock (clocks are reset rather than copied).
  //! @param other Character to copy from
  Character( const Character &other )
      : has_active_bomb( other.has_active_bomb ),
        underwater( other.underwater ),
        skip_damage_cooldown_once( other.skip_damage_cooldown_once ),
        m_damage_cooldown_timer(),
        m_global_bomb_flash_clk() // Reset clocks on copy
  {
  }

  //! @brief Custom copy assignment operator; clocks are restarted/reset rather than copied.
  //! @param other Character to copy from
  //! @return Character& reference to this instance
  Character &operator=( const Character &other )
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

  //! @brief Default constructor.
  Character() = default;

  //! @brief Timer used to drive the global bomb-flash visual effect.
  sf::Clock m_global_bomb_flash_clk;
};

} // namespace Game::Cmp::Player

#endif // SRC_COMPONENTS_PLAYER_CHARACTER_HPP__
