#ifndef SRC_COMPONENTS_RUIN_GATESEGMENT_HPP__
#define SRC_COMPONENTS_RUIN_GATESEGMENT_HPP__

namespace Game::Cmp::Ruin
{

// Segment for the StairsGateMultiBlock
class GateSegment
{
public:
  GateSegment( bool solid_mask )
      : m_solid_mask( solid_mask )
  {
  }

  [[nodiscard]] bool isSolidMask() const { return m_solid_mask; }
  void set_solid_mask( bool solid_mask ) { m_solid_mask = solid_mask; }

private:
  // Is collision detection enabled for this sprite
  bool m_solid_mask{ true };
};

} // namespace Game::Cmp::Ruin

#endif // SRC_COMPONENTS_RUIN_GATESEGMENT_HPP__
