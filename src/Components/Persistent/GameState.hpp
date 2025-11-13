#ifndef __COMPONENTS_GAME_STATE_HPP__
#define __COMPONENTS_GAME_STATE_HPP__

#include <nlohmann/json.hpp>
#include <string>
namespace ProceduralMaze::Cmp::Persistent {

// This component does not use JSON serialization as it is managed internally by the game engine.
class GameState
{
public:
  GameState() = default;
  ~GameState() = default;

  enum class State
  {
    MENU,
    SETTINGS,
    LOADING,
    PLAYING,
    PAUSED,
    GAMEOVER,
    UNLOADING,
    EXITING
  };

  State current_state = State::MENU;

  std::string class_name() const
  {
    // get the type for this class
    return "GameState";
  }

  virtual nlohmann::json serialize() const { return {}; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __COMPONENTS_GAME_STATE_HPP__