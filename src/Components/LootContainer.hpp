#ifndef __COMPONENTS_LOOTCONTAINER_HPP__
#define __COMPONENTS_LOOTCONTAINER_HPP__

#include <Sprites/MultiSprite.hpp>

namespace ProceduralMaze::Cmp
{

// Loot container concept
struct LootContainer
{
  [[maybe_unused]] bool active = true;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_LOOTCONTAINER_HPP__