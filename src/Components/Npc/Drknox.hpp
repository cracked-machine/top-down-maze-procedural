#ifndef SRC_COMPONENTS_NPC_DRKNOX_HPP__
#define SRC_COMPONENTS_NPC_DRKNOX_HPP__

namespace Game::Cmp::Npc
{

//! @brief Marker component tagging an entity as the Drknox NPC (a friendly NPC type).
struct Drknox
{
  //! @brief Always true; presence of the component is what identifies the entity as Drknox.
  bool drknox{ true };
};

} // namespace Game::Cmp::Npc

#endif // SRC_COMPONENTS_NPC_DRKNOX_HPP__
