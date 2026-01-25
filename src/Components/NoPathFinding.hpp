#ifndef SRC_COMPONENTS_NOPATHFINDING_HPP__
#define SRC_COMPONENTS_NOPATHFINDING_HPP__

namespace ProceduralMaze::Cmp
{

//! @brief Mark entity that should not be included in Npc pathfinding algorithms
struct NoPathFinding
{
  bool active{ true };
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_NOPATHFINDING_HPP__