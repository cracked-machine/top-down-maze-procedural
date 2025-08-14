#ifndef __COMPONENTS_OBSTACLE_HPP__
#define __COMPONENTS_OBSTACLE_HPP__

#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Components {

class Obstacle {
public:
    bool visible{true};
    bool enabled{true};
};

} // namespace ProceduralMaze::Components

#endif // __COMPONENTS_OBSTACLE_HPP__