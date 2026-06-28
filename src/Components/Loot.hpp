#ifndef SRC_COMPONENTS_LOOT_HPP__
#define SRC_COMPONENTS_LOOT_HPP__

#include <Factory/SpriteFactory.hpp>

namespace Game::Cmp
{

//! @brief Mark an entity as a dropped loot item
struct Loot
{
  [[maybe_unused]] bool active = true;
};

} // namespace Game::Cmp
#endif // SRC_COMPONENTS_LOOT_HPP__
