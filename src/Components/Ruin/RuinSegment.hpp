#ifndef SRC_COMPONENTS_RUINSEGMENT_HPP_
#define SRC_COMPONENTS_RUINSEGMENT_HPP_

#include <Sprites/MultiSprite.hpp>

namespace ProceduralMaze::Cmp
{

// Mainly used to distinguish between 16x16 block Ruin segments for the purpose of collision detection
class RuinSegment
{
public:
  RuinSegment( bool solid_mask )
      : m_solid_mask( solid_mask )
  {
  }

  bool isSolidMask() const { return m_solid_mask; }
  void set_solid_mask( bool solid_mask ) { m_solid_mask = solid_mask; }

private:
  // Is collision detection enabled for this sprite
  bool m_solid_mask{ true };
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_RUINSEGMENT_HPP_