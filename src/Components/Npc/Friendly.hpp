#ifndef SRC_COMPONENTS_NPC_FRIENDLY_HPP__
#define SRC_COMPONENTS_NPC_FRIENDLY_HPP__

namespace Game::Cmp::Npc
{

//! @brief Marker component tagging an NPC as non-hostile; excluded from player-collision/threat views.
struct Friendly
{
  //! @brief Always true; presence of the component is what marks the NPC as friendly.
  bool stub{ true };
};

} // namespace Game::Cmp::Npc

#endif // SRC_COMPONENTS_NPC_FRIENDLY_HPP__
