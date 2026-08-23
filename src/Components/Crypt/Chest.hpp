#ifndef SRC_COMPONENTS_CRYPT_CHEST_HPP__
#define SRC_COMPONENTS_CRYPT_CHEST_HPP__

namespace Game::Cmp::Crypt
{

//! @brief Marks an entity as a crypt loot chest that the player can open.
struct Chest
{
  //! @brief Whether the chest has been opened by the player.
  bool open{ false };
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_CHEST_HPP__
