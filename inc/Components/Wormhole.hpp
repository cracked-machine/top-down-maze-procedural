#ifndef __COMPONENTS_WORMHOLE_HPP__
#define __COMPONENTS_WORMHOLE_HPP__

#include <spdlog/spdlog.h>

#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Cmp {

class Wormhole
{
public:
  bool active{ true };
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_WORMHOLE_HPP__