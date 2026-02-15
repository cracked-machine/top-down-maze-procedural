#ifndef __COMPONENTS_LOOTCONTAINER_HPP__
#define __COMPONENTS_LOOTCONTAINER_HPP__

namespace ProceduralMaze::Cmp
{

//! @brief Mark entity that can be broken in order to drop loot
struct LootContainer
{
  int hp{ 100 };
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_LOOTCONTAINER_HPP__