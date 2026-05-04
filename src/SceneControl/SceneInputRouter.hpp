#ifndef __INPUT_EVENT_HANDLER_HPP__
#define __INPUT_EVENT_HANDLER_HPP__

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Window.hpp>

#include <SceneControl/Events/SceneManagerEvent.hpp>
#include <Systems/BaseSystem.hpp>

// clang-format off
namespace ProceduralMaze::Sprites { class SpriteFactory; }
namespace ProceduralMaze::Sys { class Store; }
namespace ProceduralMaze::Audio { class SoundBank; }
// clang-format on

namespace ProceduralMaze::Sys
{

class SceneInputRouter : public Sys::BaseSystem
{
public:
  enum class NavigationActions { NONE, TITLE, SETTINGS, CRYPT, PAUSE, RESUME, GAMEOVER, EXIT };

  SceneInputRouter( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
                    entt::dispatcher &nav_event_dispatcher, entt::dispatcher &scenemanager_event_dispatcher );

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

  void title_scene_input_handler();
  void settings_scene_state_handler();
  void graveyard_scene_state_handler();
  void crypt_scene_state_handler();
  void holywell_scene_state_handler();
  void shop_scene_state_handler();
  void ruin_scene_state_handler();
  void paused_scene_state_handler();
  void game_over_scene_state_handler();
  void level_complete_scene_state_handler();

  entt::dispatcher &m_nav_event_dispatcher;
  entt::dispatcher &m_scenemanager_event_dispatcher;

private:
  //! @brief reset and process new user direction input
  void process_move_keys();

  void resize_window( sf::Vector2u size );
  void toggle_collision_detection();
  void toggle_show_pathfinding();
  void toggle_show_debug();
  void toggle_show_nopath();
  void toggle_shaders();
  void queue_suicide_event();
  void queue_buy_item_event( uint8_t item_idx );
  void queue_quit_game_event();
  void enqueue( Events::SceneManagerEvent::Type ev );
  void toggle_particle_test( bool enable );
};

} // namespace ProceduralMaze::Sys

#endif // __INPUT_EVENT_HANDLER_HPP__