#ifndef SCENE_HOLYWELLSCENE_HPP_
#define SCENE_HOLYWELLSCENE_HPP_

#include <SFML/System/Vector2.hpp>
#include <SceneControl/Events/ProcessHolyWellSceneInputEvent.hpp>
#include <SceneControl/Scene.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Sprites/TileMap.hpp>
#include <Utils/Utils.hpp>

// clang-format off
namespace ProceduralMaze::Sys { class Store; }
namespace ProceduralMaze::Audio { class SoundBank; }
// clang-format on

namespace ProceduralMaze::Scene
{

class HolyWellScene : public Scene<Events::ProcessHolyWellSceneInputEvent>
{
public:
  HolyWellScene( Audio::SoundBank &sound_bank, Sys::Store &system_store, entt::dispatcher &nav_event_dispatcher,
                 Sprites::SpriteFactory &sprite_Factory )
      : Scene( nav_event_dispatcher ),
        m_sound_bank( sound_bank ),
        m_sys( system_store ),
        m_sprite_Factory( sprite_Factory )
  {
  }

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  std::string get_name() const override { return "HolyWellScene"; }

  entt::registry &registry() override;

protected:
  void do_update( sf::Time dt ) override;

private:
  Audio::SoundBank &m_sound_bank;
  Sys::Store &m_sys;
  Sprites::SpriteFactory &m_sprite_Factory;
  Sprites::Containers::TileMap m_floormap{};
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_HOLYWELLSCENE_HPP_