#ifndef SCENE_GRAVEYARDSCENE_HPP_
#define SCENE_GRAVEYARDSCENE_HPP_

#include <SceneControl/IScene.hpp>
#include <SystemStore.hpp>

namespace ProceduralMaze::Scene
{

class GraveyardScene : public IScene
{
public:
  GraveyardScene( Audio::SoundBank &sound_bank, Sys::SystemStore &system_store, entt::dispatcher &nav_event_dispatcher );

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  void update( [[maybe_unused]] sf::Time dt ) override;
  std::string get_name() const override { return "GraveyardScene"; }

  entt::registry &get_registry() override;

private:
  Audio::SoundBank &m_sound_bank;

  Sys::SystemStore &m_system_store;
  entt::dispatcher &m_nav_event_dispatcher;

  // restrict the path tracking data update to every 0.1 seconds (optimization)
  const sf::Time m_obstacle_distance_update_interval{ sf::milliseconds( 100 ) };
  // path tracking timer (optimization)
  sf::Clock m_obstacle_distance_timer;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_GRAVEYARDSCENE_HPP_