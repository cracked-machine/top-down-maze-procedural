#ifndef SYSTEMS_SYSTEMS_HPP_
#define SYSTEMS_SYSTEMS_HPP_

#include <EventHandler.hpp>
#include <Systems/AnimSystem.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/CollisionSystem.hpp>
#include <Systems/DiggingSystem.hpp>
#include <Systems/ExitSystem.hpp>
#include <Systems/LargeObstacleSystem.hpp>
#include <Systems/LootSystem.hpp>
#include <Systems/PathFindSystem.hpp>
#include <Systems/PersistentSystem.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/ProcGen/CellAutomataSystem.hpp>
#include <Systems/ProcGen/RandomLevelGenerator.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/Render/RenderMenuSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Systems/Threats/BombSystem.hpp>
#include <Systems/Threats/HazardFieldSystem.hpp>
#include <Systems/Threats/NpcSystem.hpp>
#include <Systems/Threats/WormholeSystem.hpp>

namespace ProceduralMaze::Sys
{

struct SystemPtrs
{
  RenderMenuSystem *render_menu_sys;
  EventHandler *event_handler;
  RenderGameSystem *render_game_sys;
  PersistentSystem *persistent_sys;
  PlayerSystem *player_sys;
  PathFindSystem *path_find_sys;
  NpcSystem *npc_sys;
  CollisionSystem *collision_sys;
  DiggingSystem *digging_sys;
  RenderOverlaySystem *render_overlay_sys;
  RenderPlayerSystem *render_player_sys;
  BombSystem *bomb_sys;
  AnimSystem *anim_sys;
  SinkHoleHazardSystem *sinkhole_sys;
  CorruptionHazardSystem *corruption_sys;
  WormholeSystem *wormhole_sys;
  ExitSystem *exit_sys;
  FootstepSystem *footstep_sys;
  LargeObstacleSystem *large_obstacle_sys;
  LootSystem *loot_sys;
};

} // namespace ProceduralMaze::Sys

#endif // SYSTEMS_SYSTEMS_HPP_