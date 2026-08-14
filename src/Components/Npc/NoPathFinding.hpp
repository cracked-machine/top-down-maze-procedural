#ifndef SRC_COMPONENTS_NPC_NOPATHFINDING_HPP__
#define SRC_COMPONENTS_NPC_NOPATHFINDING_HPP__

namespace Game::Cmp::Npc
{

//! @brief Mark entity that should not be included in Npc pathfinding algorithms
struct NoPathFinding
{
  bool active{ true };
};

} // namespace Game::Cmp::Npc

#endif // SRC_COMPONENTS_NPC_NOPATHFINDING_HPP__
