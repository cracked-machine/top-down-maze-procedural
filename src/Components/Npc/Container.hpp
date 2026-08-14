#ifndef SRC_COMPONENTS_NPC_CONTAINER_HPP__
#define SRC_COMPONENTS_NPC_CONTAINER_HPP__

namespace Game::Cmp::Npc
{

// NPC container concept
struct Container
{
  [[maybe_unused]] bool active = true;
};

} // namespace Game::Cmp::Npc

#endif // SRC_COMPONENTS_NPC_CONTAINER_HPP__
