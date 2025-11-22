#ifndef SRC_EVENTS_SCENEMANAGER_EVENT_HPP_
#define SRC_EVENTS_SCENEMANAGER_EVENT_HPP_
namespace ProceduralMaze::Events
{

struct SceneManagerEvent
{
  enum class Type
  {
    EXIT_GAME,
    SETTINGS_MENU,
    EXIT_SETTINGS_MENU,
    START_GAME,
    QUIT_GAME,
    PAUSE_GAME,
    RESUME_GAME,
    GAME_OVER,
    LEVEL_COMPLETE,
    RETURN_TO_TITLE
  };

  Type m_type;
  explicit SceneManagerEvent( Type type )
      : m_type( type )
  {
  }
};

} // namespace ProceduralMaze::Events
#endif // SRC_EVENTS_SCENEMANAGER_EVENT_HPP_