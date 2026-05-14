#ifndef SRC_CMPS_INVENTORY_PLAYERINVENTORYSLOT_HPP_
#define SRC_CMPS_INVENTORY_PLAYERINVENTORYSLOT_HPP_

#include <Components/Inventory/WorldItem.hpp>

namespace ProceduralMaze::Cmp
{

// Represents a single slot in the player inventory. If we want multiple slots we would need a "slot id",
// and this would be handled in `PlayerFactory` and `RegistryTransfer::xfer_inventory_entt`
struct PlayerInventorySlot
{
  WorldItem m_item;
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_INVENTORY_PLAYERINVENTORYSLOT_HPP_