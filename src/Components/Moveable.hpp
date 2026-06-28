#ifndef SRC_COMPONENTS_MOVEABLE_HPP__
#define SRC_COMPONENTS_MOVEABLE_HPP__

namespace Game::Cmp
{

//! @brief Used to mark an entity that can have its obstacle pushed or pulled.
struct Moveable
{
  [[maybe_unused]] bool active = true;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_MOVEABLE_HPP__
