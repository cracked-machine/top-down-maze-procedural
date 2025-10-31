#ifndef __EVENTS_PLAYER_ACTION_EVENT_HPP__
#define __EVENTS_PLAYER_ACTION_EVENT_HPP__

namespace ProceduralMaze::Events {

struct PlayerActionEvent
{
  enum class GameActions
  {
    DROP_BOMB,
    ACTIVATE,
    DIG
  };

  GameActions action;
  explicit PlayerActionEvent( GameActions act )
      : action( act )
  {
  }
};

} // namespace ProceduralMaze::Events

#endif // __EVENTS_PLAYER_ACTION_EVENT_HPP__