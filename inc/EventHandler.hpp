#ifndef __INPUT_EVENT_HANDLER_HPP__
#define __INPUT_EVENT_HANDLER_HPP__

#include <BaseSystem.hpp>
#include <Components/System.hpp>
#include <Direction.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Persistent/GameState.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Window.hpp>
#include <optional>

#include <Components/PlayableCharacter.hpp>
#include <Components/Position.hpp>

#include <spdlog/spdlog.h>

#include <imgui-SFML.h>

namespace ProceduralMaze {

class EventHandler : public Sys::BaseSystem
{

public:
  EventHandler( ProceduralMaze::SharedEnttRegistry reg );

  void menu_state_handler( sf::RenderWindow &window );
  void settings_state_handler( sf::RenderWindow &window );
  void game_state_handler( sf::RenderWindow &window );
  void paused_state_handler( sf::RenderWindow &window );
  void game_over_state_handler( sf::RenderWindow &window );
};

} // namespace ProceduralMaze

#endif // __INPUT_EVENT_HANDLER_HPP__