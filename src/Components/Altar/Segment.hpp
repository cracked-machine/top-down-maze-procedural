#ifndef SRC_COMPONENTS_ALTAR_SEGMENT_HPP__
#define SRC_COMPONENTS_ALTAR_SEGMENT_HPP__

namespace Game::Cmp::Altar
{

//! @brief Mainly used to distinguish between 16x16 block altar segments for the purpose of collision detection.
class Segment
{
public:
  //! @brief Construct a Segment with the given initial solid mask state.
  //! @param solid_mask Whether collision detection is enabled for this segment's sprite.
  Segment( bool solid_mask )
      : m_solid_mask( solid_mask )
  {
  }

  //! @brief Whether collision detection is currently enabled for this segment's sprite.
  //! @return bool True if solid (collidable).
  bool isSolidMask() const { return m_solid_mask; }
  //! @brief Set whether collision detection is enabled for this segment's sprite.
  //! @param solid_mask The new solid mask state.
  void set_solid_mask( bool solid_mask ) { m_solid_mask = solid_mask; }

private:
  //! @brief Is collision detection enabled for this sprite.
  bool m_solid_mask{ true };
};

} // namespace Game::Cmp::Altar

#endif // SRC_COMPONENTS_ALTAR_SEGMENT_HPP__
