#ifndef SRC_COMPONENTS_CRYPTINTERIORSEGMENT_HPP__
#define SRC_COMPONENTS_CRYPTINTERIORSEGMENT_HPP__

namespace ProceduralMaze::Cmp
{

// Mainly used to distinguish between 16x16 block crypt segments for the purpose of collision detection
class CryptInteriorSegment
{
public:
  CryptInteriorSegment( bool solid_mask )
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

#endif // SRC_COMPONENTS_CRYPTINTERIORSEGMENT_HPP__