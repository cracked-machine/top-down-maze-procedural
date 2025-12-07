#ifndef SCENE_CRYPTSCENE_HPP_
#define SCENE_CRYPTSCENE_HPP_

#include <SceneControl/Events/ProcessCryptSceneInputEvent.hpp>
#include <SceneControl/Scene.hpp>
#include <Systems/SystemStore.hpp>

namespace ProceduralMaze::Scene
{

class CryptScene : public Scene<Events::ProcessCryptSceneInputEvent>
{
public:
  CryptScene( Audio::SoundBank &sound_bank, Sys::SystemStore &system_store, entt::dispatcher &nav_event_dispatcher )
      : Scene( nav_event_dispatcher ),
        m_sound_bank( sound_bank ),
        m_system_store( system_store )
  {
  }

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  std::string get_name() const override { return "CryptScene"; }

  entt::registry &get_registry() override;

protected:
  void do_update( [[maybe_unused]] sf::Time dt ) override;

private:
  Audio::SoundBank &m_sound_bank;
  Sys::SystemStore &m_system_store;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_CRYPTSCENE_HPP_