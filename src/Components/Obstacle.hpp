#ifndef __COMPONENTS_OBSTACLE_HPP__
#define __COMPONENTS_OBSTACLE_HPP__

#include <spdlog/spdlog.h>

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>

#include <Components/Random.hpp>
#include <Sprites/SpriteFactory.hpp>

#include <cstdint>

namespace ProceduralMaze::Cmp {

// Obstacle concept
class Obstacle
{
public:
  Obstacle( Sprites::SpriteMetaType type, std::size_t tile_index = 0, bool enabled = true )
      : m_type( type ),
        m_tile_index( tile_index ),
        m_enabled( enabled )
  {
  }

  Sprites::SpriteMetaType m_type;
  uint32_t m_tile_index{ 0 };

  // used for procedural generation algorithm and detonation logic
  bool m_enabled{ true };
  // integrity of the obstacle (0 = destroyed, 1 = intact)
  float m_integrity{ 1.0f };
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_OBSTACLE_HPP__