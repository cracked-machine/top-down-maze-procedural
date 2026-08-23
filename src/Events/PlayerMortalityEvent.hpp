#ifndef SRC_EVENTS_PLAYERMORTALITYEVENT_HPP__
#define SRC_EVENTS_PLAYERMORTALITYEVENT_HPP__

#include <Components/Player/Mortality.hpp>
#include <Components/Position.hpp>
#include <SFML/System/Vector2.hpp>

namespace Game::Events
{

//! @brief Signals a change in the player's mortality state, e.g. dying to a hazard, drowning, or being revived.
class PlayerMortalityEvent
{
public:
  //! @brief Construct a new PlayerMortalityEvent object
  //! @param new_state The mortality state the player is transitioning to.
  //! @param death_pos Position associated with the state change, e.g. where the player died.
  PlayerMortalityEvent( Cmp::Player::Mortality::State new_state, Cmp::Position death_pos )
      : m_new_state( new_state ),
        m_death_pos( death_pos )
  {
  }
  //! @brief The mortality state the player is transitioning to.
  Cmp::Player::Mortality::State m_new_state;
  //! @brief Position associated with the state change, e.g. where the player died.
  Cmp::Position m_death_pos;
};

} // namespace Game::Events

#endif // SRC_EVENTS_PLAYERMORTALITYEVENT_HPP__
