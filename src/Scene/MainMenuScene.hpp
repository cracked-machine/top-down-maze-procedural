#ifndef SCENE_MAINMENUSCENE_HPP_
#define SCENE_MAINMENUSCENE_HPP_

#include <Audio/SoundBank.hpp>
#include <EventHandler.hpp>
#include <Systems/PersistentSystem.hpp>
#include <Systems/Render/RenderMenuSystem.hpp>

#include <Scene/IScene.hpp>

namespace ProceduralMaze::Scene
{

class MainMenuScene : public IScene
{
public:
  MainMenuScene( Audio::SoundBank &sound_bank, Sys::PersistentSystem *persistent_sys, Sys::RenderMenuSystem *render_menu_sys,
                 Sys::EventHandler *event_handler );

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  void update( [[maybe_unused]] sf::Time dt ) override;

  entt::registry *get_registry() override;

private:
  Audio::SoundBank &m_sound_bank;

  Sys::PersistentSystem *m_persistent_sys;
  Sys::RenderMenuSystem *m_render_menu_sys;
  Sys::EventHandler *m_event_handler;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_MAINMENUSCENE_HPP_