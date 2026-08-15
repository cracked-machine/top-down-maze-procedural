#ifndef SRC_COMPONENTS_CRYPT_CRYPTSEGMENT_HPP__
#define SRC_COMPONENTS_CRYPT_CRYPTSEGMENT_HPP__

namespace Game::Cmp::Crypt
{

// Mainly used to distinguish between 16x16 block crypt segments for the purpose of collision detection
class BuildingSegment
{
public:
  BuildingSegment( bool solid_mask )
      : m_solid_mask( solid_mask )
  {
  }

  bool isSolidMask() const { return m_solid_mask; }
  void set_solid_mask( bool solid_mask ) { m_solid_mask = solid_mask; }

private:
  // Is collision detection enabled for this sprite
  bool m_solid_mask{ true };
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_CRYPTSEGMENT_HPP__
