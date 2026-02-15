#ifndef SRC_COMPONENTS_RUINSTAIRSSEGMENT_HPP_
#define SRC_COMPONENTS_RUINSTAIRSSEGMENT_HPP_

namespace ProceduralMaze::Cmp
{

// Mainly used to distinguish between 16x16 block Ruin Stairs segments for the purpose of collision detection
class RuinStairsSegment
{
public:
  RuinStairsSegment( bool solid_mask )
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

#endif // SRC_COMPONENTS_RUINSTAIRSSEGMENT_HPP_