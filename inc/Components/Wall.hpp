#ifndef __COMPONENTS_WALL_HPP__
#define __COMPONENTS_WALL_HPP__

#include <spdlog/spdlog.h>

#include <Exit.hpp>
#include <Random.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <cstdint>
#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Cmp {

// Wall concept
class Wall
{
public:
  Wall( Sprites::SpriteMetaType type, std::size_t tile_index = 0 )
      : m_type( type ),
        m_tile_index( tile_index )
  {
  }
  Sprites::SpriteMetaType m_type;
  uint32_t m_tile_index{ 0 };
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_WALL_HPP__