#ifndef __COMPONENTS_WEAPON_LEVEL_HPP__
#define __COMPONENTS_WEAPON_LEVEL_HPP__

namespace ProceduralMaze::Cmp {

class WeaponLevel
{
public:
  WeaponLevel( float level )
      : m_level( level )
  {
  }
  float m_level;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_WEAPON_LEVEL_HPP__