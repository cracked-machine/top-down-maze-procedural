#ifndef SCENE_PAUSEDMENUSCENE_HPP_
#define SCENE_PAUSEDMENUSCENE_HPP_

#include <EventHandler.hpp>
#include <Scene/IScene.hpp>
#include <Systems/PersistentSystem.hpp>
#include <Systems/Render/RenderMenuSystem.hpp>
#include <entt/signal/fwd.hpp>

namespace ProceduralMaze::Scene
{

class PausedMenuScene : public IScene
{
public:
  PausedMenuScene( Audio::SoundBank &sound_bank, Sys::PersistentSystem *persistent_sys,
                   Sys::EventHandler *event_handler, Sys::RenderMenuSystem *render_menu_sys,
                   entt::dispatcher &nav_event_dispatcher );

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  void update( [[maybe_unused]] sf::Time dt ) override;
  std::string get_name() const override { return "PausedMenuScene"; }

  entt::registry *get_registry() override;

  bool blocks_update() const override { return true; }

private:
  Audio::SoundBank &m_sound_bank;

  Sys::PersistentSystem *m_persistent_sys;
  Sys::EventHandler *m_event_handler;
  Sys::RenderMenuSystem *m_render_menu_sys;

  entt::dispatcher &m_nav_event_dispatcher;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_PAUSEDMENUSCENE_HPP_