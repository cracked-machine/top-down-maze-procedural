#ifndef SRC_CMPS_NOMOVEDEST_HPP_
#define SRC_CMPS_NOMOVEDEST_HPP_

namespace Game::Cmp
{

//! @brief Used to mark an entity that cannot have an obstacle push into or its obstacle pulled from.
struct NoMoveDest
{
  [[maybe_unused]] bool active = true;
};

} // namespace Game::Cmp

#endif // SRC_CMPS_NOMOVEDEST_HPP_