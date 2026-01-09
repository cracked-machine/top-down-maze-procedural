#ifndef SRC_INVENTORY_CARRYITEM_HPP_
#define SRC_INVENTORY_CARRYITEM_HPP_

namespace ProceduralMaze::Cmp
{

//! @brief Types for the carryitems
//! @note this order should match the order in the "INVENTORY" sprite
enum class CarryItemType {
  PICKAXE, // default
  AXE,
  SHOVEL,
  NONE // always at end
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

// Represents a single slot in the player inventory
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