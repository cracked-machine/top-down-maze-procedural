#ifndef SRC_EVENTS_PLAYERMORTALITYEVENT_
#define SRC_EVENTS_PLAYERMORTALITYEVENT_

#include <Components/PlayerMortality.hpp>
#include <Components/Position.hpp>
#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Events
{

class PlayerMortalityEvent
{
public:
  PlayerMortalityEvent( Cmp::PlayerMortality::State new_state, Cmp::Position death_pos )
      : m_new_state( new_state ),
        m_death_pos( death_pos )
  {
  }
  Cmp::PlayerMortality::State m_new_state;
  Cmp::Position m_death_pos;
};

} // namespace ProceduralMaze::Events

#endif // SRC_EVENTS_PLAYERMORTALITYEVENT_