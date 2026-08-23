#ifndef SRC_COMPONENTS_NPC_WITCH_HPP__
#define SRC_COMPONENTS_NPC_WITCH_HPP__

namespace Game::Cmp::Npc
{

//! @brief Marker component tagging an entity as the Witch NPC (a hostile NPC type).
struct Witch
{
  //! @brief Always true; presence of the component is what identifies the entity as a Witch.
  bool witch{ true };
};

} // namespace Game::Cmp::Npc

#endif // SRC_COMPONENTS_NPC_WITCH_HPP__
