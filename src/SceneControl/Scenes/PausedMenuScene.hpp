#ifndef SRC_SCENECONTROL_SCENES_PAUSEDMENUSCENE_HPP__
#define SRC_SCENECONTROL_SCENES_PAUSEDMENUSCENE_HPP__

#include <SceneControl/Events/ProcessPausedMenuSceneInputEvent.hpp>
#include <SceneControl/Scene.hpp>

// clang-format off
namespace Game::Sys { class Store; }
namespace Game::Audio { class SoundBank; }
// clang-format on

namespace Game::Scene
{

class PausedMenuScene : public Scene<Events::ProcessPausedMenuSceneInputEvent>
{
public:
  PausedMenuScene( Audio::SoundBank &sound_bank, Sys::Store &system_store, entt::dispatcher &nav_event_dispatcher )
      : Scene( nav_event_dispatcher ),
        m_sound_bank( sound_bank ),
        m_sys( system_store )
  {
  }

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  std::string get_name() const override { return "PausedMenuScene"; }

  entt::registry &registry() override;

protected:
  void do_update( [[maybe_unused]] sf::Time dt ) override;

private:
  Audio::SoundBank &m_sound_bank;
  Sys::Store &m_sys;
};

} // namespace Game::Scene

#endif // SRC_SCENECONTROL_SCENES_PAUSEDMENUSCENE_HPP__
