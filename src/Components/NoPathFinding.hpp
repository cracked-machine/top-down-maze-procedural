#ifndef SRC_COMPONENTS_NOPATHFINDING_HPP__
#define SRC_COMPONENTS_NOPATHFINDING_HPP__

namespace ProceduralMaze::Cmp
{
// Component to mark entities that should be ignored by pathfinding systems
struct NoPathFinding
{

  bool active{ true };
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_NOPATHFINDING_HPP__