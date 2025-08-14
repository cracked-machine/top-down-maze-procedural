#ifndef __SYSTEMS_BASE_SYSTEM_HPP__
#define __SYSTEMS_BASE_SYSTEM_HPP__

#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Systems {

class BaseSystem {
public:
    BaseSystem() = default;
    ~BaseSystem() = default;
};

} // namespace ProceduralMaze::Systems

#endif // __SYSTEMS_BASE_SYSTEM_HPP__