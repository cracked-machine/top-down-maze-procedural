#ifndef __COMPONENTS_SYSTEM_HPP__
#define __COMPONENTS_SYSTEM_HPP__

namespace ProceduralMaze::Cmp
{

class System
{
public:
  bool collisions_disabled = false; // F1
  bool show_path_finding = false;   // F2
  bool show_debug_stats = false;    // F3
  bool show_npcnopath = false;      // F4 cycle
  bool show_playernopath = false;   // F4 cycle
  bool level_complete = false;
  bool dark_mode_enabled = true; // F9
  bool particle_test_enabled = true;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_SYSTEM_HPP__