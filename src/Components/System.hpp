#ifndef __COMPONENTS_SYSTEM_HPP__
#define __COMPONENTS_SYSTEM_HPP__

namespace ProceduralMaze::Cmp
{

class System
{
public:
  bool collisions_disabled = false; // F1
  bool show_path_finding = false;   // F5
  bool show_debug_stats = false;    // F6
  bool level_complete = false;
  bool dark_mode_enabled = true; // F9
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_SYSTEM_HPP__