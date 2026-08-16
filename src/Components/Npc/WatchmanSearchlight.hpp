#ifndef SRC_COMPONENTS_NPC_WATCHMANSEARCHLIGHT_HPP__
#define SRC_COMPONENTS_NPC_WATCHMANSEARCHLIGHT_HPP__

#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

namespace Game::Cmp::Npc
{

//! @brief Per-Watchman sweeping searchlight-cone state, plus the (per-entity) gunfire fire-rate
//! state machine, since each Watchman's cone sweeps and spots the player independently.
struct WatchmanSearchlight
{
  //! @brief Radians accumulator driving the sin() sweep; randomized at spawn so multiple
  //! Watchmen don't sweep in lockstep.
  float sweep_phase{ 0.f };

  //! @brief Last known non-zero facing angle (radians), derived from Cmp::Direction.
  //! Used as the sweep's center angle, and as the fallback when Cmp::Direction == {0,0}.
  float last_facing_angle{ 0.f };

  //! @brief Unit vector for the cone's current sweep direction, recomputed every tick.
  sf::Vector2f cone_direction{ 1.f, 0.f };

  //! @brief True once the sweeping cone has caught the player — while true the beam tracks the
  //! player directly instead of continuing its sin() sweep, until they escape cone range.
  bool locked_on_player{ false };

  //! @brief Seconds since the last idle cardinal-direction change; only advances while the
  //! Watchman is standing sentry (not locked onto and not physically moving toward the player).
  sf::Time idle_direction_timer{ sf::Time::Zero };

  //! @brief Index into the 4 cardinal directions the Watchman cycles through while idle.
  int idle_direction_index{ 0 };

  //! @brief False until the idle cardinal direction has been picked at least once.
  bool idle_direction_initialized{ false };

  //! @brief Per-entity timer, reused for both the cock->fire delay (NpcWatchmanGunFireRate) and
  //! the fire->cock reload delay (NpcWatchmanGunReloadRate) — only one phase is ever active.
  sf::Time gunfire_timer{ sf::Time::Zero };

  //! @brief True once the gun has been cocked and is counting down to fire.
  bool gun_cocked{ false };

  //! @brief False until the Watchman has fired at least once — the very first cock on initial
  //! target acquisition happens immediately (no reload delay); every cock after that waits out
  //! NpcWatchmanGunReloadRate.
  bool gun_ever_fired{ false };
};

} // namespace Game::Cmp::Npc

#endif // SRC_COMPONENTS_NPC_WATCHMANSEARCHLIGHT_HPP__
