#ifndef __CMP__SHRINE_SEGMENT_HPP__
#define __CMP__SHRINE_SEGMENT_HPP__

#include <Sprites/MultiSprite.hpp>

namespace ProceduralMaze::Cmp {

class ShrineSegment
{
public:
  ShrineSegment( bool solid_mask )
      : m_solid_mask( solid_mask )
  {
  }

  bool isSolidMask() const { return m_solid_mask; }

private:
  // Is collision detection enabled for this sprite
  bool m_solid_mask{ true };
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP__SHRINE_SEGMENT_HPP__