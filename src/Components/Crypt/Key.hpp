#ifndef SRC_COMPONENTS_CRYPT_KEY_HPP__
#define SRC_COMPONENTS_CRYPT_KEY_HPP__

namespace Game::Cmp::Crypt
{

//! @brief Marks an entity as a crypt key item that the player can collect.
struct Key
{
  //! @brief Whether the key is still active/available to collect.
  bool m_active{ true };
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_KEY_HPP__
