#ifndef __COMPONENTS_EXIT_HPP__
#define __COMPONENTS_EXIT_HPP__

namespace ProceduralMaze::Cmp {

class Exit
{
public:
  // Exit spawns randomly at the start of game but is locked. It can be unlocked later.
  bool m_locked{ true };
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_EXIT_HPP__