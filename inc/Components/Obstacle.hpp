#ifndef __COMPONENTS_OBSTACLE_HPP__
#define __COMPONENTS_OBSTACLE_HPP__

#include <Random.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <cstdint>
#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Cmp {

// Obstacle concept
class Obstacle
{
public:
  Obstacle( Sprites::SpriteFactory::Type type, std::size_t tile_index = 0, bool visible = true, bool enabled = true )
      : m_type( type ), m_tile_index( tile_index ), m_visible( visible ), m_enabled( enabled )
  {
  }

  Sprites::SpriteFactory::Type m_type;
  uint32_t m_tile_index{ 0 };

  bool m_visible{ true };
  // if true this obstacle will be considered impassable
  bool m_enabled{ true };
  bool m_broken{ false };
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_OBSTACLE_HPP__