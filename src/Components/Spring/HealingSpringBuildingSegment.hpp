#ifndef SRC_COMPONENTS_SPRING_SACREDSPRINGBUILDINGSEGMENT_HPP__
#define SRC_COMPONENTS_SPRING_SACREDSPRINGBUILDINGSEGMENT_HPP__

namespace Game::Cmp
{

//! @brief Mainly used to distinguish between 16x16 block healing spring building segments for the
//! purpose of collision detection.
class HealingSpringBuildingSegment
{
public:
  //! @brief Construct a new HealingSpringBuildingSegment object.
  //! @param solid_mask Whether collision detection is enabled for this sprite.
  HealingSpringBuildingSegment( bool solid_mask )
      : m_solid_mask( solid_mask )
  {
  }

  //! @brief Check whether collision detection is enabled for this segment.
  //! @return true if collision detection is enabled.
  //! @return false otherwise.
  [[nodiscard]] bool isSolidMask() const { return m_solid_mask; }

  //! @brief Set whether collision detection is enabled for this segment.
  //! @param solid_mask true to enable collision detection.
  void set_solid_mask( bool solid_mask ) { m_solid_mask = solid_mask; }

private:
  //! @brief Is collision detection enabled for this sprite.
  bool m_solid_mask{ true };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_SPRING_SACREDSPRINGBUILDINGSEGMENT_HPP__
