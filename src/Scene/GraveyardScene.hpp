#ifndef SCENE_GRAVEYARDSCENE_HPP_
#define SCENE_GRAVEYARDSCENE_HPP_

#include <EventHandler.hpp>
#include <Scene/IScene.hpp>
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
#include <entt/signal/fwd.hpp>

namespace ProceduralMaze::Scene
{

class GraveyardScene : public IScene
{
public:
  GraveyardScene( Audio::SoundBank &sound_bank, Sys::PersistentSystem *persistent_sys, Sys::PlayerSystem *player_sys,
                  Sys::RenderGameSystem *render_game_sys, Sys::EventHandler *event_handler, Sys::AnimSystem *anim_sys,
                  Sys::SinkHoleHazardSystem *sinkhole_sys, Sys::CorruptionHazardSystem *corruption_sys,
                  Sys::BombSystem *bomb_sys, Sys::ExitSystem *exit_sys, Sys::LootSystem *loot_sys,
                  Sys::NpcSystem *npc_sys, Sys::WormholeSystem *wormhole_sys, Sys::DiggingSystem *digging_sys,
                  Sys::FootstepSystem *footstep_sys, Sys::PathFindSystem *path_find_sys,
                  Sys::RenderOverlaySystem *render_overlay_sys, Sys::RenderPlayerSystem *render_player_sys,
                  Sys::ProcGen::RandomLevelGenerator *random_level_sys,
                  Sys::ProcGen::CellAutomataSystem *cellauto_parser, entt::dispatcher &nav_event_dispatcher );

  void on_init() override;
  void on_enter() override;
  void on_exit() override;
  void update( [[maybe_unused]] sf::Time dt ) override;
  std::string get_name() const override { return "GraveyardScene"; }

  entt::registry *get_registry() override;

private:
  Audio::SoundBank &m_sound_bank;

  Sys::PersistentSystem *m_persistent_sys;
  Sys::PlayerSystem *m_player_sys;
  Sys::EventHandler *m_event_handler;
  Sys::RenderGameSystem *m_render_game_sys;
  Sys::AnimSystem *m_anim_sys;
  Sys::SinkHoleHazardSystem *m_sinkhole_sys;
  Sys::CorruptionHazardSystem *m_corruption_sys;
  Sys::BombSystem *m_bomb_sys;
  Sys::ExitSystem *m_exit_sys;
  Sys::LootSystem *m_loot_sys;
  Sys::NpcSystem *m_npc_sys;
  Sys::WormholeSystem *m_wormhole_sys;
  Sys::DiggingSystem *m_digging_sys;
  Sys::FootstepSystem *m_footstep_sys;
  Sys::PathFindSystem *m_path_find_sys;
  Sys::RenderOverlaySystem *m_render_overlay_sys;
  Sys::RenderPlayerSystem *m_render_player_sys;
  Sys::ProcGen::RandomLevelGenerator *random_level_sys;
  Sys::ProcGen::CellAutomataSystem *cellauto_parser;

  // restrict the path tracking data update to every 0.1 seconds (optimization)
  const sf::Time m_obstacle_distance_update_interval{ sf::milliseconds( 100 ) };
  // path tracking timer (optimization)
  sf::Clock m_obstacle_distance_timer;

  entt::dispatcher &m_nav_event_dispatcher;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_GRAVEYARDSCENE_HPP_