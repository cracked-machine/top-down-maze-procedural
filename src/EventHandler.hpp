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
  enum class NavigationActions
  {
    NONE,
    TITLE,
    SETTINGS,
    PLAY,
    PAUSE,
    RESUME,
    GAMEOVER,
    EXIT
  };

  EventHandler( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                Audio::SoundBank &sound_bank );

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  NavigationActions menu_state_handler();
  NavigationActions settings_state_handler();
  NavigationActions game_state_handler();
  NavigationActions paused_state_handler();
  NavigationActions game_over_state_handler();
};

} // namespace ProceduralMaze::Sys

#endif // __INPUT_EVENT_HANDLER_HPP__