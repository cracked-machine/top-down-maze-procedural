#ifndef SRC_COMPONENTS_PLANTSEGMENT_HPP__
#define SRC_COMPONENTS_PLANTSEGMENT_HPP__

namespace Game::Cmp
{

//! @brief Mainly used to distinguish between 16x16 block plant segments for the purpose of collision
//! detection.
class PlantSegment
{
public:
  //! @brief Construct a new PlantSegment object.
  //! @param solid_mask Whether collision detection is enabled for this segment.
  PlantSegment( bool solid_mask )
      : m_solid_mask( solid_mask )
  {
  }

  //! @brief Check whether collision detection is enabled for this segment.
  //! @return bool
  bool isSolidMask() const { return m_solid_mask; }

  //! @brief Set whether collision detection is enabled for this segment.
  //! @param solid_mask true to enable collision detection.
  void set_solid_mask( bool solid_mask ) { m_solid_mask = solid_mask; }

private:
  //! @brief Is collision detection enabled for this segment.
  bool m_solid_mask{ true };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_PLANTSEGMENT_HPP__
