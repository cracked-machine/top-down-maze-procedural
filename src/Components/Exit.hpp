#ifndef SRC_COMPONENTS_EXIT_HPP__
#define SRC_COMPONENTS_EXIT_HPP__

namespace Game::Cmp
{

//! @brief Marks the entity that is the level's exit tile.
class Exit
{
public:
  //! @brief Exit spawns randomly at the start of the game but is locked. It can be unlocked later.
  bool m_locked{ true };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_EXIT_HPP__
