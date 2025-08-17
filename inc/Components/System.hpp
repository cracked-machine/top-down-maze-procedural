#ifndef __COMPONENTS_SYSTEM_HPP__
#define __COMPONENTS_SYSTEM_HPP__

#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Cmp {

class System {
public:
    bool local_view = false;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_SYSTEM_HPP__