#ifndef __COMPONENTS_WALL_HPP__
#define __COMPONENTS_WALL_HPP__

namespace Game::Cmp
{

// Wall concept
struct Wall
{
  [[maybe_unused]] bool blocking = true;
};

} // namespace Game::Cmp

#endif // __COMPONENTS_WALL_HPP__