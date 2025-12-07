#ifndef SCENE_SETTINGSMENUSCENE_HPP_
#define SCENE_SETTINGSMENUSCENE_HPP_

#include <SceneControl/Events/ProcessSettingsMenuSceneInputEvent.hpp>
#include <SceneControl/Scene.hpp>
#include <Systems/SystemStore.hpp>

namespace ProceduralMaze::Scene
{

class SettingsMenuScene : public Scene<Events::ProcessSettingsMenuSceneInputEvent>
{
public:
  SettingsMenuScene( Sys::SystemStore &system_store, entt::dispatcher &nav_event_dispatcher )
      : Scene( nav_event_dispatcher ),
        m_system_store( system_store )
  {
  }

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  std::string get_name() const override { return "SettingsMenuScene"; }

  entt::registry &get_registry() override;

protected:
  void do_update( [[maybe_unused]] sf::Time dt ) override;

private:
  Sys::SystemStore &m_system_store;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_SETTINGSMENUSCENE_HPP_