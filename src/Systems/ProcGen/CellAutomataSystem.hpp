#ifndef __SYSTEMS_PROCGEN_CELLAUTO_SYSTEM_HPP__
#define __SYSTEMS_PROCGEN_CELLAUTO_SYSTEM_HPP__

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>

#include <spdlog/spdlog.h>

#include <Components/Neighbours.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Position.hpp>
#include <Components/Random.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/ProcGen/RandomLevelGenerator.hpp>

#include <memory>

namespace ProceduralMaze::Sys::ProcGen
{

class CellAutomataSystem : public BaseSystem
{
public:
  CellAutomataSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                      Audio::SoundBank &sound_bank )
      : BaseSystem( reg, window, sprite_factory, sound_bank )
  {
  }

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  void iterate( unsigned int iterations );

  void set_random_level_generator( RandomLevelGenerator *random_level ) { m_random_level = random_level; }

private:
  RandomLevelGenerator *m_random_level;

  void find_neighbours();

  void apply_rules();
};

} // namespace ProceduralMaze::Sys::ProcGen

#endif // __SYSTEMS_PROCGEN_CELLAUTO_SYSTEM_HPP__