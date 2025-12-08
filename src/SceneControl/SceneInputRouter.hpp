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

class SceneInputRouter : public Sys::BaseSystem
{
public:
  enum class NavigationActions { NONE, TITLE, SETTINGS, CRYPT, PAUSE, RESUME, GAMEOVER, EXIT };

  SceneInputRouter( entt::registry &reg, sf::RenderWindow &window,
                    Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
                    entt::dispatcher &nav_event_dispatcher,
                    entt::dispatcher &scenemanager_event_dispatcher );

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  void title_scene_input_handler();
  void settings_scene_state_handler();
  void graveyard_scene_state_handler();
  void crypt_scene_state_handler();
  void paused_scene_state_handler();
  void game_over_scene_state_handler();
  void level_complete_scene_state_handler();

  entt::dispatcher &m_nav_event_dispatcher;
  entt::dispatcher &m_scenemanager_event_dispatcher;
};

} // namespace ProceduralMaze::Sys

#endif // __INPUT_EVENT_HANDLER_HPP__