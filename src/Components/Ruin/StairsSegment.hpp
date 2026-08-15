#ifndef SRC_COMPONENTS_RUIN_STAIRSSEGMENT_HPP__
#define SRC_COMPONENTS_RUIN_STAIRSSEGMENT_HPP__

namespace Game::Cmp::Ruin
{

// Mainly used to distinguish between 16x16 block Ruin Stairs segments for the purpose of collision detection
class StairsSegment
{
public:
  StairsSegment( bool solid_mask )
      : m_solid_mask( solid_mask )
  {
  }

  bool isSolidMask() const { return m_solid_mask; }
  void set_solid_mask( bool solid_mask ) { m_solid_mask = solid_mask; }

private:
  // Is collision detection enabled for this sprite
  bool m_solid_mask{ true };
};

} // namespace Game::Cmp::Ruin

#endif // SRC_COMPONENTS_RUIN_STAIRSSEGMENT_HPP__
