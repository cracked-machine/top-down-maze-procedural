#ifndef SRC_SCENECONTROL_SCENES_RUINSCENELOWERFLOOR_HPP__
#define SRC_SCENECONTROL_SCENES_RUINSCENELOWERFLOOR_HPP__

#include <Audio/SoundBank.hpp>
#include <Factory/SpriteFactory.hpp>
#include <SceneControl/Events/ProcessRuinSceneLowerInputEvent.hpp>
#include <SceneControl/Scene.hpp>
#include <Sprites/SpriteMetaType.hpp>
#include <Sprites/VertexFloor.hpp>
#include <Systems/Stores/SystemStore.hpp>

namespace Game::Sprites
{
class SpriteSheet;
class SpriteFactory;
} // namespace Game::Sprites

namespace Game::Scene
{

class RuinSceneLowerFloor : public Scene<Events::ProcessRuinSceneLowerInputEvent>
{
public:
  enum class EntryMode { FROM_DOOR, FROM_UPPER_FLOOR };

  RuinSceneLowerFloor( Audio::SoundBank &sound_bank, Sys::Store &system_store, entt::dispatcher &nav_event_dispatcher,
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
  std::string get_name() const override { return "RuinSceneLowerFloor"; }

  entt::registry &registry() override;

  void set_entry_mode( EntryMode entry_mode ) { m_entry_mode = entry_mode; }

protected:
  void do_update( [[maybe_unused]] sf::Time dt ) override;

private:
  void reinit_navmesh();

  Audio::SoundBank &m_sound_bank;
  Sys::Store &m_sys;
  Sprites::SpriteFactory &m_sprite_factory;

  EntryMode m_entry_mode;
};

} // namespace Game::Scene

#endif // SRC_SCENECONTROL_SCENES_RUINSCENELOWERFLOOR_HPP__
