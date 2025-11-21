#ifndef SCENE_SETTINGSMENUSCENE_HPP_
#define SCENE_SETTINGSMENUSCENE_HPP_

#include <EventHandler.hpp>
#include <Scene/IScene.hpp>
#include <SystemStore.hpp>
#include <Systems/PersistentSystem.hpp>
#include <Systems/Render/RenderMenuSystem.hpp>

namespace ProceduralMaze::Scene
{

class SettingsMenuScene : public IScene
{
public:
  SettingsMenuScene( Sys::SystemStore &system_store );

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  void update( [[maybe_unused]] sf::Time dt ) override;
  std::string get_name() const override { return "SettingsMenuScene"; }

  entt::registry &get_registry() override;

private:
  Sys::SystemStore &m_system_store;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_SETTINGSMENUSCENE_HPP_