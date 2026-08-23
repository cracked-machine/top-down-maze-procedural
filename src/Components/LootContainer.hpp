#ifndef SRC_COMPONENTS_LOOTCONTAINER_HPP__
#define SRC_COMPONENTS_LOOTCONTAINER_HPP__

namespace Game::Cmp
{

//! @brief Mark entity that can be broken in order to drop loot
struct LootContainer
{
  //! @brief Remaining hit points before the container breaks and drops its loot.
  int hp{ 100 };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_LOOTCONTAINER_HPP__
