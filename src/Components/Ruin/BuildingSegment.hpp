#ifndef SRC_COMPONENTS_RUIN_RUINSEGMENT_HPP__
#define SRC_COMPONENTS_RUIN_RUINSEGMENT_HPP__

namespace Game::Cmp::Ruin
{

//! @brief Marks a single 16x16 px block of a Cmp::Ruin::BuildingMultiBlock, mainly used to distinguish
//! between individual building segments for the purpose of collision detection.
class BuildingSegment
{
public:
  //! @brief Construct a new segment.
  //! @param solid_mask Initial collision-enabled state for this segment.
  BuildingSegment( bool solid_mask )
      : m_solid_mask( solid_mask )
  {
  }

  //! @brief Get whether collision detection is currently enabled for this segment.
  //! @return bool
  bool isSolidMask() const { return m_solid_mask; }
  //! @brief Set whether collision detection is enabled for this segment.
  //! @param solid_mask
  void set_solid_mask( bool solid_mask ) { m_solid_mask = solid_mask; }

private:
  //! @brief Is collision detection enabled for this sprite.
  bool m_solid_mask{ true };
};

} // namespace Game::Cmp::Ruin

#endif // SRC_COMPONENTS_RUIN_RUINSEGMENT_HPP__
