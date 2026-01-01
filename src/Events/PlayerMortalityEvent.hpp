#ifndef SRC_EVENTS_PLAYERMORTALITYEVENT_
#define SRC_EVENTS_PLAYERMORTALITYEVENT_

#include <Components/PlayerMortality.hpp>

namespace ProceduralMaze::Events
{

class PlayerMortalityEvent
{
public:
  PlayerMortalityEvent( Cmp::PlayerMortality::State new_state )
      : m_new_state( new_state )
  {
  }
  Cmp::PlayerMortality::State m_new_state;
};

} // namespace ProceduralMaze::Events

#endif // SRC_EVENTS_PLAYERMORTALITYEVENT_