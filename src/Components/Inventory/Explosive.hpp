#ifndef SRC_COMPONENTS_INVENTORY_EXPLOSIVE_HPP__
#define SRC_COMPONENTS_INVENTORY_EXPLOSIVE_HPP__

namespace Game::Cmp
{

//! @brief Marks a world/inventory item as an explosive (bomb) and tracks whether it has been armed.
struct Explosive
{
  //! @brief Whether the explosive has been armed and is counting down to detonation.
  bool armed{ false };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_INVENTORY_EXPLOSIVE_HPP__
