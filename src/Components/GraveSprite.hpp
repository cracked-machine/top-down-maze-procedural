#ifndef __CMP__GRAVE_SPRITE_HPP__
#define __CMP__GRAVE_SPRITE_HPP__

#include <Sprites/MultiSprite.hpp>
namespace ProceduralMaze::Cmp {

class GraveSprite
{
public:
  GraveSprite( Sprites::SpriteMetaType type, std::size_t tile_index, bool solid_mask )
      : m_type( type ),
        m_tile_index( tile_index ),
        m_solid_mask( solid_mask )
  {
  }

  Sprites::SpriteMetaType getType() const { return m_type; }
  std::size_t getTileIndex() const { return m_tile_index; }
  void setTileIndex( std::size_t index ) { m_tile_index = index; }
  bool isSolidMask() const { return m_solid_mask; }

private:
  Sprites::SpriteMetaType m_type;
  std::size_t m_tile_index;
  // Is collision detection enabled for this sprite
  bool m_solid_mask{ true };
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP__GRAVE_SPRITE_HPP__