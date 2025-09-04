#ifndef __COMPONENTS_PLAYERDISTANCE_HPP__
#define __COMPONENTS_PLAYERDISTANCE_HPP__

#include <entt/entity/fwd.hpp>

namespace ProceduralMaze::Cmp {

struct PlayerDistance
{
  unsigned int distance = std::numeric_limits<unsigned int>::max();
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_PLAYERDISTANCE_HPP__