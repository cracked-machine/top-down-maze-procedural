#ifndef SRC_COMPONENTS_NPC_SKELETON_HPP__
#define SRC_COMPONENTS_NPC_SKELETON_HPP__

namespace Game::Cmp::Npc
{

//! @brief Marker component tagging an entity as the Skeleton NPC (a hostile NPC type).
struct Skeleton
{
  //! @brief Always true; presence of the component is what identifies the entity as a Skeleton.
  bool skeleton{ true };
};

} // namespace Game::Cmp::Npc

#endif // SRC_COMPONENTS_NPC_SKELETON_HPP__
