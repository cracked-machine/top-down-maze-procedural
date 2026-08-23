#ifndef SRC_COMPONENTS_SPAWNAREA_HPP__
#define SRC_COMPONENTS_SPAWNAREA_HPP__

namespace Game::Cmp
{

//! @brief Marks a tile/entity as a valid spawn location for procedural level generation.
class SpawnArea
{
public:
  //! @brief Construct a spawn area marker.
  //! @param solid_mask Whether collision detection is enabled for this spawn area.
  SpawnArea( bool solid_mask )
      : m_solid_mask( solid_mask )
  {
  }

  //! @brief Returns whether collision detection is enabled for this spawn area.
  bool isSolidMask() const { return m_solid_mask; }

private:
  //! @brief Is collision detection enabled for this sprite.
  bool m_solid_mask{ true };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_SPAWNAREA_HPP__
