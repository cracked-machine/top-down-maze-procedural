#ifndef SRC_COMPONENTS_INVENTORY_PLAYERINVENTORYSLOT_HPP__
#define SRC_COMPONENTS_INVENTORY_PLAYERINVENTORYSLOT_HPP__

#include <Components/Inventory/WorldItem.hpp>

namespace Game::Cmp
{

//! @brief Represents a single slot in the player inventory.
//! @note If we want multiple slots we would need a "slot id", and this would be handled in
//! `PlayerFactory` and `RegistryTransfer::xfer_inventory_entt`.
struct PlayerInventorySlot
{
  //! @brief The item held in this inventory slot.
  WorldItem m_item;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_INVENTORY_PLAYERINVENTORYSLOT_HPP__
