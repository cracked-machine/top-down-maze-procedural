#ifndef SRC_COMPONENTS_GRAVE_SEGMENT_HPP__
#define SRC_COMPONENTS_GRAVE_SEGMENT_HPP__

namespace Game::Cmp::Grave
{

// Mainly used to distinguish between 16x16 block grave segments for the purpose of collision detection
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
  // Is collision detection enabled for this segment
  bool m_solid_mask{ true };
};

} // namespace Game::Cmp::Grave

#endif // SRC_COMPONENTS_GRAVE_SEGMENT_HPP__
