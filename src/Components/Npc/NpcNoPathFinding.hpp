#ifndef SRC_COMPONENTS_NPC_NPCNOPATHFINDING_HPP__
#define SRC_COMPONENTS_NPC_NPCNOPATHFINDING_HPP__

namespace Game::Cmp
{

//! @brief Mark entity that should not be included in Npc pathfinding algorithms
struct NpcNoPathFinding
{
  bool active{ true };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_NPC_NPCNOPATHFINDING_HPP__
