#ifndef SRC_COMPONENTS_SPRING_SACREDSPRINGSEGMENT_HPP__
#define SRC_COMPONENTS_SPRING_SACREDSPRINGSEGMENT_HPP__

namespace Game::Cmp
{

class HealingSpringSegment
{
public:
  HealingSpringSegment( bool solid_mask )
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

#endif // SRC_COMPONENTS_SPRING_SACREDSPRINGSEGMENT_HPP__
