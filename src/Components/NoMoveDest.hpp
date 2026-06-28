#ifndef SRC_COMPONENTS_NOMOVEDEST_HPP__
#define SRC_COMPONENTS_NOMOVEDEST_HPP__

namespace Game::Cmp
{

//! @brief Used to mark an entity that cannot have an obstacle push into or its obstacle pulled from.
struct NoMoveDest
{
  [[maybe_unused]] bool active = true;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_NOMOVEDEST_HPP__
