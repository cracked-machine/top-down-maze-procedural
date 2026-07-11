#ifndef SRC_COMPONENTS_WELL_FOUNTAINSEGMENT_HPP__
#define SRC_COMPONENTS_WELL_FOUNTAINSEGMENT_HPP__

namespace Game::Cmp
{

class FountainSegment
{
public:
  FountainSegment( bool solid_mask )
      : m_solid_mask( solid_mask )
  {
  }

  bool isSolidMask() const { return m_solid_mask; }
  void set_solid_mask( bool solid_mask ) { m_solid_mask = solid_mask; }

private:
  // Is collision detection enabled for this sprite
  bool m_solid_mask{ true };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_WELL_FOUNTAINSEGMENT_HPP__
