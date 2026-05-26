#ifndef SRC_CMPS_MOVEABLE_HPP_
#define SRC_CMPS_MOVEABLE_HPP_

namespace Game::Cmp
{

//! @brief Used to mark an entity that can have its obstacle pushed or pulled.
struct Moveable
{
  [[maybe_unused]] bool active = true;
};

} // namespace Game::Cmp

#endif // SRC_CMPS_MOVEABLE_HPP_