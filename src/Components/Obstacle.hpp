#ifndef __COMPONENTS_OBSTACLE_HPP__
#define __COMPONENTS_OBSTACLE_HPP__

#include <spdlog/spdlog.h>

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>

#include <Components/Random.hpp>
#include <Sprites/SpriteFactory.hpp>

namespace ProceduralMaze::Cmp
{

// Mark the entity as a Rock obstacle
struct Obstacle
{

  bool m_enabled{ true };
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_OBSTACLE_HPP__