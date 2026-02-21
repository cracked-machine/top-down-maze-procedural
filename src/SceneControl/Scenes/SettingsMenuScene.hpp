#ifndef SCENE_SETTINGSMENUSCENE_HPP_
#define SCENE_SETTINGSMENUSCENE_HPP_

#include <SceneControl/Events/ProcessSettingsMenuSceneInputEvent.hpp>
#include <SceneControl/Scene.hpp>

// clang-format off
namespace ProceduralMaze::Sys { class Store; }
namespace ProceduralMaze::Audio { class SoundBank; }
// clang-format on

namespace ProceduralMaze::Scene
{

class SettingsMenuScene : public Scene<Events::ProcessSettingsMenuSceneInputEvent>
{
public:
  SettingsMenuScene( Sys::Store &system_store, entt::dispatcher &nav_event_dispatcher )
      : Scene( nav_event_dispatcher ),
        m_sys( system_store )
  {
  }

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  std::string get_name() const override { return "SettingsMenuScene"; }

  entt::registry &registry() override;

protected:
  void do_update( [[maybe_unused]] sf::Time dt ) override;

private:
  Sys::Store &m_sys;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_SETTINGSMENUSCENE_HPP_