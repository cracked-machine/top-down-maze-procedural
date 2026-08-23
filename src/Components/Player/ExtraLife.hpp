#ifndef SRC_COMPONENTS_PLAYER_EXTRALIFE_HPP__
#define SRC_COMPONENTS_PLAYER_EXTRALIFE_HPP__

namespace Game::Cmp::Player
{

//! @brief Marks the player as having an extra life available, preventing death when active.
struct ExtraLife
{
  //! @brief Whether the extra life is currently available/active.
  [[maybe_unused]] bool active{ true };
};

} // namespace Game::Cmp::Player

#endif // SRC_COMPONENTS_PLAYER_EXTRALIFE_HPP__
