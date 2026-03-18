#ifndef SCENE_CRYPTSCENE_HPP_
#define SCENE_CRYPTSCENE_HPP_

#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>
#include <SceneControl/Events/ProcessCryptSceneInputEvent.hpp>
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

class CryptScene : public Scene<Events::ProcessCryptSceneInputEvent>
{
public:
  CryptScene( Audio::SoundBank &sound_bank, Sys::Store &system_store, entt::dispatcher &nav_event_dispatcher, Sprites::SpriteFactory &sprite_Factory )
      : Scene( nav_event_dispatcher ),
        m_sound_bank( sound_bank ),
        m_sys( system_store ),
        m_sprite_Factory( sprite_Factory )
  {
  }

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  std::string get_name() const override { return "CryptScene"; }

  entt::registry &registry() override;

  static sf::Clock &get_maze_timer() { return s_maze_timer; }
  static bool is_maze_timer_expired() { return s_maze_timer.getElapsedTime() > s_maze_timer_cooldown ? true : false; }
  static void stop_maze_timer() { s_maze_timer.reset(); }

protected:
  void do_update( [[maybe_unused]] sf::Time dt ) override;

private:
  Audio::SoundBank &m_sound_bank;
  Sys::Store &m_sys;
  Sprites::SpriteFactory &m_sprite_Factory;
  Sprites::Containers::TileMap m_floormap{};

  static sf::Clock s_maze_timer;
  constexpr static sf::Time s_maze_timer_cooldown{ sf::seconds( 10.f ) };
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_CRYPTSCENE_HPP_