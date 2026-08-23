#ifndef SRC_COMPONENTS_EXITKEY_HPP__
#define SRC_COMPONENTS_EXITKEY_HPP__

namespace Game::Cmp
{

//! @brief Marks an entity as a key that can be collected to help unlock the level's Cmp::Exit.
struct ExitKey
{
  //! @brief Whether the key is currently active (e.g. still present/collectible in the world).
  bool m_active{ true };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_EXITKEY_HPP__
