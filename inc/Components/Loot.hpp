#ifndef __COMPONENTS_LOOT_HPP__
#define __COMPONENTS_LOOT_HPP__

#include <SpriteFactory.hpp>

namespace ProceduralMaze::Cmp {

class Loot
{
public:
  Loot( Sprites::SpriteFactory::Type type, std::size_t tile_index ) : m_type( type ), m_tile_index( tile_index ) {}
  Sprites::SpriteFactory::Type m_type;
  std::size_t m_tile_index;
};

} // namespace ProceduralMaze::Cmp
#endif // __COMPONENTS_LOOT_HPP__