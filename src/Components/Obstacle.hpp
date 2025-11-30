#ifndef __COMPONENTS_OBSTACLE_HPP__
#define __COMPONENTS_OBSTACLE_HPP__

#include <spdlog/spdlog.h>

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>

#include <Components/Random.hpp>
#include <Sprites/SpriteFactory.hpp>

namespace ProceduralMaze::Cmp
{

// Obstacle concept
class Obstacle
{
public:
  Obstacle( bool enabled = true )
      : m_enabled( enabled )
  {
  }

  // used for procedural generation algorithm and detonation logic
  bool m_enabled{ true };
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_OBSTACLE_HPP__