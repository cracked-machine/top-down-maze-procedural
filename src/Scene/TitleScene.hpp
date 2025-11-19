#ifndef SCENE_TITLESCENE_HPP_
#define SCENE_TITLESCENE_HPP_

#include <Audio/SoundBank.hpp>
#include <EventHandler.hpp>
#include <Systems/PersistentSystem.hpp>
#include <Systems/Render/RenderMenuSystem.hpp>

#include <Scene/IScene.hpp>

namespace ProceduralMaze::Scene
{

class TitleScene : public IScene
{
public:
  TitleScene( Audio::SoundBank &sound_bank, Sys::PersistentSystem *persistent_sys,
              Sys::RenderMenuSystem *render_menu_sys, Sys::EventHandler *event_handler );

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  void update( [[maybe_unused]] sf::Time dt ) override;
  std::string get_name() const override { return "MainMenuScene"; }

  entt::registry *get_registry() override;

private:
  Audio::SoundBank &m_sound_bank;

  Sys::PersistentSystem *m_persistent_sys;
  Sys::RenderMenuSystem *m_render_menu_sys;
  Sys::EventHandler *m_event_handler;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_TITLESCENE_HPP_