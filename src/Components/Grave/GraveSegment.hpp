#ifndef __CMP__GRAVE_SEGMENT_HPP__
#define __CMP__GRAVE_SEGMENT_HPP__

#include <Sprites/MultiSprite.hpp>
namespace ProceduralMaze::Cmp
{

// Mainly used to distinguish between 16x16 block grave segments for the purpose of collision detection
class GraveSegment
{
public:
  GraveSegment( bool solid_mask )
      : m_solid_mask( solid_mask )
  {
  }

  bool isSolidMask() const { return m_solid_mask; }

private:
  // Is collision detection enabled for this segment
  bool m_solid_mask{ true };
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP__GRAVE_SEGMENT_HPP__