#ifndef SRC_COMPONENTS_CRYPTSEGMENT_HPP__
#define SRC_COMPONENTS_CRYPTSEGMENT_HPP__

#include <Sprites/MultiSprite.hpp>

namespace ProceduralMaze::Cmp
{

// Mainly used to distinguish between 16x16 block crypt segments for the purpose of collision detection
class CryptSegment
{
public:
  CryptSegment( bool solid_mask )
      : m_solid_mask( solid_mask )
  {
  }

  bool isSolidMask() const { return m_solid_mask; }

private:
  // Is collision detection enabled for this sprite
  bool m_solid_mask{ true };
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_CRYPTSEGMENT_HPP__