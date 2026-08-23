#ifndef SRC_COMPONENTS_WALL_HPP__
#define SRC_COMPONENTS_WALL_HPP__

namespace Game::Cmp
{

//! @brief Marks the entity as an impassable wall tile.
struct Wall
{
  //! @brief Whether this wall is currently active/blocking.
  bool active = true;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_WALL_HPP__
