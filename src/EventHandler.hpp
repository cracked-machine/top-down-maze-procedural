#ifndef __INPUT_EVENT_HANDLER_HPP__
#define __INPUT_EVENT_HANDLER_HPP__

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Window.hpp>

#include <imgui-SFML.h>
#include <spdlog/spdlog.h>

#include <Components/Direction.hpp>
#include <Components/Persistent/GameState.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Position.hpp>
#include <Components/System.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sys
{

class EventHandler : public Sys::BaseSystem
{
public:
  enum class MenuAction
  {
    NONE,
    MENU,
    SETTINGS,
    LOAD,
    PLAY,
    PAUSE,
    GAMEOVER,
    UNLOAD,
    EXIT
  };

  EventHandler( sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  MenuAction menu_state_handler();
  MenuAction settings_state_handler();
  void game_state_handler();
  void paused_state_handler();
  void game_over_state_handler();
};

} // namespace ProceduralMaze::Sys

#endif // __INPUT_EVENT_HANDLER_HPP__