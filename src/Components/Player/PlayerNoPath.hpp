#ifndef SRC_COMPONENTS_PLAYERNOPATH_HPP__
#define SRC_COMPONENTS_PLAYERNOPATH_HPP__

namespace ProceduralMaze::Cmp
{

//! @brief Mark entity that blocks player movement
struct PlayerNoPath
{
  bool active{ true };
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_PLAYERNOPATH_HPP__