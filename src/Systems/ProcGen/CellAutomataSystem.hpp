#ifndef __SYSTEMS_PROCGEN_CELLAUTO_SYSTEM_HPP__
#define __SYSTEMS_PROCGEN_CELLAUTO_SYSTEM_HPP__

#include <SpatialHashGrid.hpp>
#include <entt/entity/fwd.hpp>

#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>

#include <Systems/BaseSystem.hpp>
#include <Systems/ProcGen/RandomLevelGenerator.hpp>

// fwd declarations
namespace Components
{
class Neighbours;
class Obstacle;
class Position;
class Random;
} // namespace Components

namespace ProceduralMaze::Sys::ProcGen
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

} // namespace ProceduralMaze::Sys::ProcGen

#endif // __SYSTEMS_PROCGEN_CELLAUTO_SYSTEM_HPP__