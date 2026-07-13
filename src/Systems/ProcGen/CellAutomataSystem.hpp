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

class CellAutomataSystem : public BaseSystem
{
public:
  CellAutomataSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
      : BaseSystem( reg, window, sprite_factory, sound_bank )
  {
  }

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

  void iterate( uint16_t iterations, uint8_t birth_threshold, uint8_t survival_threshold, LevelGenerator::SceneType scene_type,
                PathFinding::SpatialHashGrid &levelgen_spatialgrid, PathFinding::SpatialHashGridSharedPtr reserved_navmesh );
};

} // namespace Game::Sys::ProcGen

#endif // SRC_SYSTEMS_PROCGEN_CELLAUTOMATASYSTEM_HPP__
