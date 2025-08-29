#ifndef __COMPONENTS_SYSTEM_HPP__
#define __COMPONENTS_SYSTEM_HPP__

#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Cmp {

class System {
public:
    bool collisions_enabled = true;
    bool show_armed_obstacles = false;
    bool show_obstacle_entity_id = false;
    bool show_pathfinding = false;
    bool show_dijkstra_distance = false;
    bool pause_flood = false;
    bool player_stuck = false;
    bool level_complete = false;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_SYSTEM_HPP__