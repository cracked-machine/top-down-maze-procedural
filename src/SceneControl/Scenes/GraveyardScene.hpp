#ifndef SCENE_GRAVEYARDSCENE_HPP_
#define SCENE_GRAVEYARDSCENE_HPP_

#include <SFML/System/Time.hpp>
#include <SceneControl/Events/ProcessGraveyardSceneInputEvent.hpp>
#include <SceneControl/Scene.hpp>

#include <Sprites/TileMap.hpp>
#include <Systems/BaseSystem.hpp>
#include <Utils/Utils.hpp>

// clang-format off
namespace ProceduralMaze::Sys { class Store; }
namespace ProceduralMaze::Audio { class SoundBank; }
// clang-format on

namespace ProceduralMaze::Scene
{

class GraveyardScene : public Scene<Events::ProcessGraveyardSceneInputEvent>
{
public:
  GraveyardScene( Audio::SoundBank &sound_bank, Sys::Store &system_store, entt::dispatcher &nav_event_dispatcher,
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
  std::string get_name() const override { return "GraveyardScene"; }

  entt::registry &registry() override;

protected:
  void do_update( sf::Time dt ) override;

private:
  Audio::SoundBank &m_sound_bank;
  Sys::Store &m_sys;
  Sprites::SpriteFactory &m_sprite_factory;

  Sprites::Containers::TileMap m_floormap;
  sf::Clock m_scene_exit_cooldown{};
  sf::Time m_scene_exit_cooldown_time{ sf::seconds( 2 ) };

  void reinit_navmesh();
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_GRAVEYARDSCENE_HPP_