#ifndef __COMPONENTS_EXIT_HPP__
#define __COMPONENTS_EXIT_HPP__

#include <entt/entity/fwd.hpp>

namespace ProceduralMaze::Cmp {

class Exit
{
public:
private:
  // This is redundant but ENTT requires components have at least one member
  [[maybe_unused]] const bool m_is_exit{ true };
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_EXIT_HPP__