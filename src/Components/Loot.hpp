#ifndef __COMPONENTS_LOOT_HPP__
#define __COMPONENTS_LOOT_HPP__

#include <Sprites/SpriteFactory.hpp>

namespace ProceduralMaze::Cmp {

class Loot
{
public:
  Loot( Sprites::SpriteMetaType type, std::size_t tile_index )
      : m_type( type ),
        m_tile_index( tile_index )
  {
  }
  Sprites::SpriteMetaType m_type;
  std::size_t m_tile_index;
};

} // namespace ProceduralMaze::Cmp
#endif // __COMPONENTS_LOOT_HPP__