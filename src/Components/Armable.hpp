#ifndef SRC_COMPONENTS_ARMABLE_HPP__
#define SRC_COMPONENTS_ARMABLE_HPP__

namespace Game::Cmp
{

//! @brief Component that indicates an entity can be armed by bomb
struct Armable
{
  //! @brief Whether this entity is currently eligible to be armed by a bomb.
  [[maybe_unused]] bool armable{ true };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_ARMABLE_HPP__
