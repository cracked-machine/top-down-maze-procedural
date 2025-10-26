#ifndef __COMPONENTS_LOOTCONTAINER_HPP__
#define __COMPONENTS_LOOTCONTAINER_HPP__

#include <MultiSprite.hpp>

namespace ProceduralMaze::Cmp {

// Loot container concept
class LootContainer
{
public:
  LootContainer( Sprites::SpriteMetaType type, std::size_t tile_index = 0 )
      : m_type( type ),
        m_tile_index( tile_index )
  {
  }

  Sprites::SpriteMetaType m_type;
  unsigned int m_tile_index{ 0 };
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_LOOTCONTAINER_HPP__