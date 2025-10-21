#ifndef __EVENTS_PLAYER_ACTION_EVENT_HPP__
#define __EVENTS_PLAYER_ACTION_EVENT_HPP__

#include <SFML/System/Vector2.hpp>
#include <entt/entity/entity.hpp>

namespace ProceduralMaze::Events {

struct PlayerActionEvent
{
  enum class GameActions
  {
    DROP_BOMB,
    DIG_GRAVE,
    ACTIVATE_SHRINE
  };

  GameActions action;
  explicit PlayerActionEvent( GameActions act )
      : action( act )
  {
  }
};

} // namespace ProceduralMaze::Events

#endif // __EVENTS_PLAYER_ACTION_EVENT_HPP__