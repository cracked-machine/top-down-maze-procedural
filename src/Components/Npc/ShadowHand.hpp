#ifndef SRC_COMPONENTS_NPC_SHADOWHAND_HPP__
#define SRC_COMPONENTS_NPC_SHADOWHAND_HPP__

namespace Game::Cmp::Npc
{

//! @brief Marker component tagging an entity as the ShadowHand NPC (a hostile NPC type).
struct ShadowHand
{
  //! @brief Always true; presence of the component is what identifies the entity as a ShadowHand.
  bool shadowhand{ true };
};

} // namespace Game::Cmp::Npc

#endif // SRC_COMPONENTS_NPC_SHADOWHAND_HPP__
