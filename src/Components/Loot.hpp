#ifndef __COMPONENTS_LOOT_HPP__
#define __COMPONENTS_LOOT_HPP__

#include <Sprites/SpriteFactory.hpp>

namespace ProceduralMaze::Cmp
{

//! @brief Mark an entity as a dropped loot item
struct Loot
{
  [[maybe_unused]] bool active = true;
};

} // namespace ProceduralMaze::Cmp
#endif // __COMPONENTS_LOOT_HPP__