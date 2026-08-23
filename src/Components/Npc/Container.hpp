#ifndef SRC_COMPONENTS_NPC_CONTAINER_HPP__
#define SRC_COMPONENTS_NPC_CONTAINER_HPP__

namespace Game::Cmp::Npc
{

//! @brief Marks an entity as a lazily-spawned NPC container (e.g. a bones pile), which spawns a
//! real NPC when interacted with rather than existing as a live NPC itself.
struct Container
{
  //! @brief Always true while the entity is an active container; presence of the component is what matters.
  [[maybe_unused]] bool active = true;
};

} // namespace Game::Cmp::Npc

#endif // SRC_COMPONENTS_NPC_CONTAINER_HPP__
