#ifndef SRC_COMPONENTS_NONPCPATHFINDING_HPP__
#define SRC_COMPONENTS_NONPCPATHFINDING_HPP__

namespace ProceduralMaze::Cmp
{

//! @brief Mark entity that should not be included in Npc pathfinding algorithms
struct NpcNoPathFinding
{
  bool active{ true };
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_NONPCPATHFINDING_HPP__