#ifndef SCENE_SETTINGSMENUSCENE_HPP_
#define SCENE_SETTINGSMENUSCENE_HPP_

#include <EventHandler.hpp>
#include <Scene/IScene.hpp>
#include <Systems/PersistentSystem.hpp>
#include <Systems/Render/RenderMenuSystem.hpp>

namespace ProceduralMaze::Scene
{

class SettingsMenuScene : public IScene
{
public:
  SettingsMenuScene( Sys::PersistentSystem *persistent_sys, Sys::RenderMenuSystem *render_menu_sys,
                     Sys::EventHandler *event_handler );

  void on_enter() override;
  void on_exit() override;
  void update( [[maybe_unused]] sf::Time dt ) override;

  entt::registry *get_registry() override;

private:
  Sys::PersistentSystem *m_persistent_sys;
  Sys::RenderMenuSystem *m_render_menu_sys;
  Sys::EventHandler *m_event_handler;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_SETTINGSMENUSCENE_HPP_