#ifndef SCENE_GAMEOVERSCENE_HPP_
#define SCENE_GAMEOVERSCENE_HPP_

#include <SceneControl/Events/ProcessGameoverSceneInputEvent.hpp>
#include <SceneControl/Scene.hpp>

// clang-format off
namespace ProceduralMaze::Sys { class Store; }
namespace ProceduralMaze::Audio { class SoundBank; }
// clang-format on

namespace ProceduralMaze::Scene
{

class GameOverScene : public Scene<Events::ProcessGameoverSceneInputEvent>
{
public:
  GameOverScene( Audio::SoundBank &sound_bank, Sys::Store &system_store, entt::dispatcher &nav_event_dispatcher )
      : Scene( nav_event_dispatcher ),
        m_sound_bank( sound_bank ),
        m_sys( system_store )
  {
  }

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  std::string get_name() const override { return "GameOverScene"; }

  entt::registry &registry() override;

protected:
  void do_update( [[maybe_unused]] sf::Time dt ) override;

private:
  Audio::SoundBank &m_sound_bank;
  Sys::Store &m_sys;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_GAMEOVERSCENE_HPP_