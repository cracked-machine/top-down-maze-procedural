#ifndef SRC_EVENTS_PLAYERMORTALITYEVENT_HPP__
#define SRC_EVENTS_PLAYERMORTALITYEVENT_HPP__

#include <Components/Player/Mortality.hpp>
#include <Components/Position.hpp>
#include <SFML/System/Vector2.hpp>

namespace Game::Events
{

class PlayerMortalityEvent
{
public:
  PlayerMortalityEvent( Cmp::Player::Mortality::State new_state, Cmp::Position death_pos )
      : m_new_state( new_state ),
        m_death_pos( death_pos )
  {
  }
  Cmp::Player::Mortality::State m_new_state;
  Cmp::Position m_death_pos;
};

} // namespace Game::Events

#endif // SRC_EVENTS_PLAYERMORTALITYEVENT_HPP__
