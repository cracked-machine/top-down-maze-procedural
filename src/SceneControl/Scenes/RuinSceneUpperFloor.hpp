#ifndef SRC_SCENECONTROL_SCENE_RUINSCENEUPPER_HPP_
#define SRC_SCENECONTROL_SCENE_RUINSCENEUPPER_HPP_

#include <Audio/SoundBank.hpp>
#include <SFML/System/Vector2.hpp>
#include <SceneControl/Events/ProcessRuinSceneUpperInputEvent.hpp>
#include <SceneControl/Scene.hpp>
#include <Sprites/TileMap.hpp>
#include <Systems/SystemStore.hpp>
#include <Utils/Utils.hpp>

namespace ProceduralMaze::Scene
{

class RuinSceneUpperFloor : public Scene<Events::ProcessRuinSceneUpperInputEvent>
{
public:
  RuinSceneUpperFloor( Audio::SoundBank &sound_bank, Sys::Store &system_store, entt::dispatcher &nav_event_dispatcher,
                       Sprites::SpriteFactory &sprite_Factory )
      : Scene( nav_event_dispatcher ),
        m_sound_bank( sound_bank ),
        m_sys( system_store ),
        m_sprite_factory( sprite_Factory )
  {
  }

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  std::string get_name() const override { return "RuinSceneUpperFloor"; }

  entt::registry &registry() override;

protected:
  void do_update( sf::Time dt ) override;

private:
  Audio::SoundBank &m_sound_bank;
  Sys::Store &m_sys;
  Sprites::SpriteFactory &m_sprite_factory;
};

} // namespace ProceduralMaze::Scene

#endif // SRC_SCENECONTROL_SCENE_RUINSCENEUPPER_HPP_