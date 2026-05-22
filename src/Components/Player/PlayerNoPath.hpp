#ifndef SRC_COMPONENTS_PLAYERNOPATH_HPP__
#define SRC_COMPONENTS_PLAYERNOPATH_HPP__

namespace Game::Cmp
{

//! @brief Mark entity that blocks player movement
struct PlayerNoPath
{
  bool active{ true };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_PLAYERNOPATH_HPP__