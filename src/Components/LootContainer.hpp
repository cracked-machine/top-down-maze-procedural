#ifndef __COMPONENTS_LOOTCONTAINER_HPP__
#define __COMPONENTS_LOOTCONTAINER_HPP__

namespace Game::Cmp
{

//! @brief Mark entity that can be broken in order to drop loot
struct LootContainer
{
  int hp{ 100 };
};

} // namespace Game::Cmp

#endif // __COMPONENTS_LOOTCONTAINER_HPP__