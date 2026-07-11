#ifndef SRC_SCENECONTROL_SCENES_SACREDSPRINGSCENE_HPP__
#define SRC_SCENECONTROL_SCENES_SACREDSPRINGSCENE_HPP__

#include <Factory/SpriteFactory.hpp>
#include <SFML/System/Vector2.hpp>
#include <SceneControl/Events/ProcessHealingSpringSceneInputEvent.hpp>
#include <SceneControl/Scene.hpp>
#include <Sprites/VertexFloor.hpp>

// clang-format off
namespace Game::Sys { class Store; }
namespace Game::Audio { class SoundBank; }
// clang-format on

namespace Game::Scene
{

class HealingSpringScene : public Scene<Events::ProcessHealingSpringSceneInputEvent>
{
public:
  HealingSpringScene( Audio::SoundBank &sound_bank, Sys::Store &system_store, entt::dispatcher &nav_event_dispatcher,
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
  [[nodiscard]] std::string get_name() const override { return "HealingSpringScene"; }

  entt::registry &registry() override;

protected:
  void do_update( sf::Time dt ) override;

private:
  void reinit_navmesh();

  Audio::SoundBank &m_sound_bank;
  Sys::Store &m_sys;
  Sprites::SpriteFactory &m_sprite_factory;
};

} // namespace Game::Scene

#endif // SRC_SCENECONTROL_SCENES_SACREDSPRINGSCENE_HPP__
