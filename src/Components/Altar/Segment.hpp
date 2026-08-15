#ifndef SRC_COMPONENTS_ALTAR_SEGMENT_HPP__
#define SRC_COMPONENTS_ALTAR_SEGMENT_HPP__

namespace Game::Cmp::Altar
{

// Mainly used to distinguish between 16x16 block altars segments for the purpose of collision detection
class Segment
{
public:
  Segment( bool solid_mask )
      : m_solid_mask( solid_mask )
  {
  }

  bool isSolidMask() const { return m_solid_mask; }
  void set_solid_mask( bool solid_mask ) { m_solid_mask = solid_mask; }

private:
  // Is collision detection enabled for this sprite
  bool m_solid_mask{ true };
};

} // namespace Game::Cmp::Altar

#endif // SRC_COMPONENTS_ALTAR_SEGMENT_HPP__
