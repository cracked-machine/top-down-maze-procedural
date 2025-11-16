#ifndef SCENE_PAUSEDMENUSCENE_HPP_
#define SCENE_PAUSEDMENUSCENE_HPP_

#include <EventHandler.hpp>
#include <Scene/IScene.hpp>
#include <Systems/PersistentSystem.hpp>
#include <Systems/Render/RenderMenuSystem.hpp>

namespace ProceduralMaze::Scene
{

class PausedMenuScene : public IScene
{
public:
  PausedMenuScene( Audio::SoundBank &sound_bank, Sys::PersistentSystem *persistent_sys, Sys::EventHandler *event_handler,
                   Sys::RenderMenuSystem *render_menu_sys );

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  void update( [[maybe_unused]] sf::Time dt ) override;

  entt::registry *get_registry() override;

  bool blocks_update() const override { return true; }

private:
  Audio::SoundBank &m_sound_bank;

  Sys::PersistentSystem *m_persistent_sys;
  Sys::EventHandler *m_event_handler;
  Sys::RenderMenuSystem *m_render_menu_sys;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_PAUSEDMENUSCENE_HPP_