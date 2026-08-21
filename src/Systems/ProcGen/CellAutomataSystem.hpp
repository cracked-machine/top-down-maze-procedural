#ifndef SRC_SYSTEMS_PROCGEN_CELLAUTOMATASYSTEM_HPP__
#define SRC_SYSTEMS_PROCGEN_CELLAUTOMATASYSTEM_HPP__

#include <Systems/BaseSystem.hpp>
#include <Systems/ProcGen/LevelGenerator.hpp>

#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>
#include <entt/entity/fwd.hpp>

// clang-format off
namespace Components { class Neighbours; class Obstacle; class Position; class Random; }
namespace PathFinding { class SpatialHashGrid; }
// clang-format on

namespace Game::Sys::ProcGen
{

//! @brief Applies a cellular-automata pass over the level's position grid, turning obstacles on/off
//!        based on neighbour counts to smooth out proc-gen noise into organic-looking cave/wall shapes.
class CellAutomataSystem : public BaseSystem
{
public:
  //! @brief Construct a new Cell Automata System object
  CellAutomataSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
      : BaseSystem( reg, window, sprite_factory, sound_bank )
  {
  }

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

  //! @brief Run the cellular-automata birth/survival rule over every non-reserved position for the given
  //!        number of iterations, adding or removing obstacles depending on live-neighbour counts.
  //! @param iterations Number of passes to run over the grid.
  //! @param birth_threshold Minimum alive-neighbour count for a dead cell to become an obstacle.
  //! @param survival_threshold Minimum alive-neighbour count for an already-alive cell to remain an obstacle.
  //! @param scene_type Determines which obstacle/pathfinding behaviour to apply (e.g. graveyard vs ruin).
  //! @param levelgen_spatialgrid
  //! @param reserved_navmesh
  void iterate( uint16_t iterations, uint8_t birth_threshold, uint8_t survival_threshold, LevelGenerator::SceneType scene_type,
                PathFinding::SpatialHashGrid &levelgen_spatialgrid, PathFinding::SpatialHashGridSharedPtr reserved_navmesh );
};

} // namespace Game::Sys::ProcGen

#endif // SRC_SYSTEMS_PROCGEN_CELLAUTOMATASYSTEM_HPP__
