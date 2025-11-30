#ifndef __COMPONENTS_NPCCONTAINER_HPP__
#define __COMPONENTS_NPCCONTAINER_HPP__

#include <Sprites/MultiSprite.hpp>

namespace ProceduralMaze::Cmp
{

// NPC container concept
struct NpcContainer
{
  [[maybe_unused]] bool active = true;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_NPCCONTAINER_HPP__