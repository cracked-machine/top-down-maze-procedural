#ifndef __COMPONENTS_NPCCONTAINER_HPP__
#define __COMPONENTS_NPCCONTAINER_HPP__

#include <MultiSprite.hpp>

namespace ProceduralMaze::Cmp {

// NPC container concept
class NpcContainer
{
public:
  NpcContainer( Sprites::SpriteMetaType type, std::size_t tile_index )
      : m_type( type ),
        m_tile_index( tile_index )
  {
  }

  Sprites::SpriteMetaType m_type;
  unsigned int m_tile_index;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_NPCCONTAINER_HPP__