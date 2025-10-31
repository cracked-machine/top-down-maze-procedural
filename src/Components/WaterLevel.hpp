#ifndef __COMPONENTS_WATER_LEVEL_HPP__
#define __COMPONENTS_WATER_LEVEL_HPP__

namespace ProceduralMaze::Cmp {

class WaterLevel
{
public:
  WaterLevel( unsigned int level )
      : m_level( level )
  {
  }
  ~WaterLevel() = default;
  float m_level;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_WATER_LEVEL_HPP__