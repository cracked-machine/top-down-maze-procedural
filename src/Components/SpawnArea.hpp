#ifndef SRC_COMPONENTS_SPAWNAREA_HPP__
#define SRC_COMPONENTS_SPAWNAREA_HPP__

namespace ProceduralMaze::Cmp
{

class SpawnArea
{
public:
  SpawnArea( bool solid_mask )
      : m_solid_mask( solid_mask )
  {
  }

  bool isSolidMask() const { return m_solid_mask; }

private:
  // Is collision detection enabled for this sprite
  bool m_solid_mask{ true };
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_SPAWNAREA_HPP__