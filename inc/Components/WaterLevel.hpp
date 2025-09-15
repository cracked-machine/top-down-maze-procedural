#ifndef __COMPONENTS_WATER_LEVEL_HPP__
#define __COMPONENTS_WATER_LEVEL_HPP__

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

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