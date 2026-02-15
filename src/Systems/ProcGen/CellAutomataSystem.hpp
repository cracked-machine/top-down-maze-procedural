#ifndef __SYSTEMS_PROCGEN_CELLAUTO_SYSTEM_HPP__
#define __SYSTEMS_PROCGEN_CELLAUTO_SYSTEM_HPP__

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
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  void iterate( unsigned int iterations, const sf::Vector2u kMapGridSize, RandomLevelGenerator::SceneType scene_type );

  void set_random_level_generator( RandomLevelGenerator *random_level ) { m_random_level = random_level; }

private:
  RandomLevelGenerator *m_random_level;

  void find_neighbours( const sf::Vector2u kMapGridSize );

  void apply_rules( RandomLevelGenerator::SceneType scene_type );
};

} // namespace ProceduralMaze::Sys::ProcGen

#endif // __SYSTEMS_PROCGEN_CELLAUTO_SYSTEM_HPP__