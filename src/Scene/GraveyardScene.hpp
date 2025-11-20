#ifndef SCENE_GRAVEYARDSCENE_HPP_
#define SCENE_GRAVEYARDSCENE_HPP_

#include <EventHandler.hpp>
#include <Scene/IScene.hpp>
#include <SystemStore.hpp>
#include <Systems/AnimSystem.hpp>
#include <Systems/DiggingSystem.hpp>
#include <Systems/ExitSystem.hpp>
#include <Systems/FootstepSystem.hpp>
#include <Systems/LootSystem.hpp>
#include <Systems/PathFindSystem.hpp>
#include <Systems/PersistentSystem.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/ProcGen/CellAutomataSystem.hpp>
#include <Systems/ProcGen/RandomLevelGenerator.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/Threats/BombSystem.hpp>
#include <Systems/Threats/NpcSystem.hpp>
#include <Systems/Threats/WormholeSystem.hpp>

namespace ProceduralMaze::Scene
{

class GraveyardScene : public IScene
{
public:
  GraveyardScene( Audio::SoundBank &sound_bank, Sys::SystemStore &system_store );

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  void update( [[maybe_unused]] sf::Time dt ) override;
  std::string get_name() const override { return "GraveyardScene"; }

  entt::registry *get_registry() override;

private:
  Audio::SoundBank &m_sound_bank;

  Sys::SystemStore &m_system_store;

  // restrict the path tracking data update to every 0.1 seconds (optimization)
  const sf::Time m_obstacle_distance_update_interval{ sf::milliseconds( 100 ) };
  // path tracking timer (optimization)
  sf::Clock m_obstacle_distance_timer;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_GRAVEYARDSCENE_HPP_