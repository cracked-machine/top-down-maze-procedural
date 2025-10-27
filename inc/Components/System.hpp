#ifndef __COMPONENTS_SYSTEM_HPP__
#define __COMPONENTS_SYSTEM_HPP__

#include <spdlog/spdlog.h>

#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Cmp {

class System
{
public:
  bool collisions_enabled = true;    // F1
  bool pause_flood = false;          // F2
  bool show_path_distances = false;  // F3
  bool show_armed_obstacles = false; // F4
  bool show_debug_stats = false;     // F5
  bool minimap_enabled = false;
  bool player_stuck = false;
  bool level_complete = false;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_SYSTEM_HPP__