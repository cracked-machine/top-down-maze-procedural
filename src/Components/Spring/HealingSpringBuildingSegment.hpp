#ifndef SRC_COMPONENTS_SPRING_SACREDSPRINGBUILDINGSEGMENT_HPP__
#define SRC_COMPONENTS_SPRING_SACREDSPRINGBUILDINGSEGMENT_HPP__

namespace Game::Cmp
{

// Mainly used to distinguish between 16x16 block  spring segments for the purpose of collision detection
class HealingSpringBuildingSegment
{
public:
  HealingSpringBuildingSegment( bool solid_mask )
      : m_solid_mask( solid_mask )
  {
  }

  [[nodiscard]] bool isSolidMask() const { return m_solid_mask; }
  void set_solid_mask( bool solid_mask ) { m_solid_mask = solid_mask; }

private:
  // Is collision detection enabled for this sprite
  bool m_solid_mask{ true };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_SPRING_SACREDSPRINGBUILDINGSEGMENT_HPP__
