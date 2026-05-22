#ifndef SRC_SYSTEMS_PROCGEN_CELLAUTOMATASYSTEM_HPP_
#define SRC_SYSTEMS_PROCGEN_CELLAUTOMATASYSTEM_HPP_

#include <Systems/BaseSystem.hpp>
#include <Systems/ProcGen/RandomLevelGenerator.hpp>

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

  void iterate( unsigned int iterations, RandomLevelGenerator::SceneType scene_type, PathFinding::SpatialHashGrid &levelgen_spatialgrid );
};

} // namespace Game::Sys::ProcGen

#endif // SRC_SYSTEMS_PROCGEN_CELLAUTOMATASYSTEM_HPP_