#ifndef SRC_COMPONENTS_NPC_WISP_HPP__
#define SRC_COMPONENTS_NPC_WISP_HPP__

namespace Game::Cmp::Npc
{

//! @brief Marker component tagging an entity as the Wisp NPC (a friendly NPC type).
struct Wisp
{
  //! @brief Always true; presence of the component is what identifies the entity as a Wisp.
  bool wisp{ true };
};

} // namespace Game::Cmp::Npc

#endif // SRC_COMPONENTS_NPC_WISP_HPP__
