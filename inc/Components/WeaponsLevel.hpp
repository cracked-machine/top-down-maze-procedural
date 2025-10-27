#ifndef __COMPONENTS_WEAPONS_LEVEL_HPP__
#define __COMPONENTS_WEAPONS_LEVEL_HPP__

#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

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