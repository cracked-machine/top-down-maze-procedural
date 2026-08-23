#ifndef SRC_COMPONENTS_GRAVE_EXITSEGMENT_HPP__
#define SRC_COMPONENTS_GRAVE_EXITSEGMENT_HPP__

namespace Game::Cmp::Grave
{

//! @brief Mainly used to distinguish between 16x16 block level-exit segments for the purpose of
//! collision detection.
class ExitSegment
{
public:
  //! @brief Construct a new ExitSegment object.
  //! @param solid_mask Whether collision detection is enabled for this segment.
  ExitSegment( bool solid_mask )
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
  //! @brief Is collision detection enabled for this segment.
  bool m_solid_mask{ true };
};

} // namespace Game::Cmp::Grave

#endif // SRC_COMPONENTS_GRAVE_EXITSEGMENT_HPP__
