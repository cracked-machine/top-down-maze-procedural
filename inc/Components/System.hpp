#ifndef __COMPONENTS_SYSTEM_HPP__
#define __COMPONENTS_SYSTEM_HPP__

#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Cmp {

class System {
public:
    bool collisions_enabled = true;
    bool show_player_hitboxes = false;
    bool show_obstacle_entity_id = false;
    bool player_stuck = false;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_SYSTEM_HPP__