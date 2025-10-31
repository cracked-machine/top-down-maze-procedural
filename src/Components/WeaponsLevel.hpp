#ifndef __COMPONENTS_WEAPONS_LEVEL_HPP__
#define __COMPONENTS_WEAPONS_LEVEL_HPP__

namespace ProceduralMaze::Cmp {

class WeaponsLevel
{
public:
  WeaponsLevel( unsigned int level )
      : m_level( level )
  {
  }
  float m_level;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_WATER_LEVEL_HPP__