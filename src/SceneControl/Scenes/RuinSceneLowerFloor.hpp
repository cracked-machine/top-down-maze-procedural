#ifndef SRC_SCENECONTROL_SCENE_RUINSCENELOWER_HPP_
#define SRC_SCENECONTROL_SCENE_RUINSCENELOWER_HPP_

#include <Audio/SoundBank.hpp>
#include <SFML/System/Vector2.hpp>
#include <SceneControl/Events/ProcessRuinSceneLowerInputEvent.hpp>
#include <SceneControl/Scene.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Sprites/TileMap.hpp>
#include <Systems/SystemStore.hpp>
#include <Utils/Utils.hpp>

#include <Sprites/SpriteMetaType.hpp>
namespace ProceduralMaze::Sprites
{
class MultiSprite;
class SpriteFactory;
} // namespace ProceduralMaze::Sprites

namespace ProceduralMaze::Scene
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
  Audio::SoundBank &m_sound_bank;
  Sys::Store &m_sys;
  Sprites::SpriteFactory &m_sprite_factory;

  EntryMode m_entry_mode;
};

} // namespace ProceduralMaze::Scene

#endif // SRC_SCENECONTROL_SCENE_RUINSCENELOWER_HPP_