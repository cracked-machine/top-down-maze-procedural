#ifndef SCENE_LEVELCOMPLETE_SCENE_HPP_
#define SCENE_LEVELCOMPLETE_SCENE_HPP_

#include <Audio/SoundBank.hpp>
#include <EventHandler.hpp>
#include <Scene/IScene.hpp>
#include <Systems/PersistentSystem.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/Render/RenderMenuSystem.hpp>

namespace ProceduralMaze::Scene
{

class LevelCompleteScene : public IScene
{
public:
  LevelCompleteScene( Audio::SoundBank &sound_bank, Sys::PersistentSystem *persistent_sys,
                      Sys::PlayerSystem *player_sys, Sys::RenderMenuSystem *render_menu_sys,
                      Sys::EventHandler *event_handler, Sys::RenderGameSystem *render_game_sys );

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  void update( [[maybe_unused]] sf::Time dt ) override;
  std::string get_name() const override { return "LevelCompleteScene"; }

  entt::registry *get_registry() override;

private:
  Audio::SoundBank &m_sound_bank;
  Sys::PersistentSystem *m_persistent_sys;
  Sys::PlayerSystem *m_player_sys;
  Sys::RenderMenuSystem *m_render_menu_sys;
  Sys::EventHandler *m_event_handler;
  Sys::RenderGameSystem *m_render_game_sys;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_LEVELCOMPLETE_SCENE_HPP_