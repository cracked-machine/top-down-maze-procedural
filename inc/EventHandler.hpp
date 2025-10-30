#ifndef __INPUT_EVENT_HANDLER_HPP__
#define __INPUT_EVENT_HANDLER_HPP__

#include <imgui-SFML.h>
#include <spdlog/spdlog.h>

#include <BaseSystem.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Position.hpp>
#include <Components/System.hpp>
#include <Direction.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Persistent/GameState.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Window.hpp>
#include <optional>

namespace ProceduralMaze::Sys {

class EventHandler : public Sys::BaseSystem
{
public:
  EventHandler( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory );

  void menu_state_handler();
  void settings_state_handler();
  void game_state_handler();
  void paused_state_handler();
  void game_over_state_handler();
};

} // namespace ProceduralMaze::Sys

#endif // __INPUT_EVENT_HANDLER_HPP__