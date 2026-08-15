#ifndef SRC_COMPONENTS_GRAVE_EXITSEGMENT_HPP__
#define SRC_COMPONENTS_GRAVE_EXITSEGMENT_HPP__

namespace Game::Cmp::Grave
{

class ExitSegment
{
public:
  ExitSegment( bool solid_mask )
      : m_solid_mask( solid_mask )
  {
  }

  [[nodiscard]] bool isSolidMask() const { return m_solid_mask; }
  void set_solid_mask( bool solid_mask ) { m_solid_mask = solid_mask; }

private:
  // Is collision detection enabled for this segment
  bool m_solid_mask{ true };
};

} // namespace Game::Cmp::Grave

#endif // SRC_COMPONENTS_GRAVE_EXITSEGMENT_HPP__
