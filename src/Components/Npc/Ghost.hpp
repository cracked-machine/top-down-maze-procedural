#ifndef SRC_COMPONENTS_NPC_GHOST_HPP__
#define SRC_COMPONENTS_NPC_GHOST_HPP__

namespace Game::Cmp::Npc
{

//! @brief Marker component tagging an entity as the Ghost NPC (a hostile NPC type).
struct Ghost
{
  //! @brief Always true; presence of the component is what identifies the entity as a Ghost.
  bool ghost{ true };
};

} // namespace Game::Cmp::Npc

#endif // SRC_COMPONENTS_NPC_GHOST_HPP__
