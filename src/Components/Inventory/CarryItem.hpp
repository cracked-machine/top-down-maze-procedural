#ifndef SRC_INVENTORY_CARRYITEM_HPP_
#define SRC_INVENTORY_CARRYITEM_HPP_

#include <vector>
namespace ProceduralMaze::Cmp
{

//! @brief Types for the carryitems
//! @note this order must match the order in the "INVENTORY" sprite
enum class CarryItemType {
  PICKAXE, // default
  AXE,
  SHOVEL,
  BOMB,
  RESERVED1,
  RESERVED2,
  PLANT1,
  PLANT2,
  PLANT3,
  PLANT4,
  PLANT5,
  PLANT6,
  PLANT7,
  PLANT8,
  PLANT9,
  PLANT10,
  PLANT11,
  PLANT12,
  NONE // always at end
};

const static std::vector<Cmp::CarryItemType> kCarryItemTypeList{
    Cmp::CarryItemType::PICKAXE,   Cmp::CarryItemType::AXE,       Cmp::CarryItemType::SHOVEL, Cmp::CarryItemType::BOMB,
    Cmp::CarryItemType::RESERVED1, Cmp::CarryItemType::RESERVED2, Cmp::CarryItemType::PLANT1, Cmp::CarryItemType::PLANT2,
    Cmp::CarryItemType::PLANT3,    Cmp::CarryItemType::PLANT4,    Cmp::CarryItemType::PLANT5, Cmp::CarryItemType::PLANT6,
    Cmp::CarryItemType::PLANT7,    Cmp::CarryItemType::PLANT8,    Cmp::CarryItemType::PLANT9, Cmp::CarryItemType::PLANT10,
    Cmp::CarryItemType::PLANT11,   Cmp::CarryItemType::PLANT12,
};

// Keeping the type generic allows easy qeury/view on all components in the game
class CarryItem
{
public:
  CarryItem( CarryItemType type )
      : type( type )
  {
  }

  CarryItemType type;
};

// Represents a single slot in the player inventory. If we want multiple slots we would need a "slot id",
// and this would be handled in `PlayerFactory` and `RegistryTransfer::xfer_inventory_entt`
class PlayerInventorySlot : public CarryItem
{
public:
  PlayerInventorySlot( CarryItemType type )
      : CarryItem( type )
  {
  }
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_INVENTORY_CARRYITEM_HPP_