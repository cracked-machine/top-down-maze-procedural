#ifndef SRC_INVENTORY_CARRYITEM_HPP_
#define SRC_INVENTORY_CARRYITEM_HPP_

#include <Sprites/MultiSprite.hpp>

namespace ProceduralMaze::Cmp
{

// Keeping the type generic allows easy qeury/view on all components in the game
class CarryItem
{
public:
  CarryItem( Sprites::SpriteMetaType type )
      : type( type )
  {
  }

  Sprites::SpriteMetaType type;
};

// Represents a single slot in the player inventory. If we want multiple slots we would need a "slot id",
// and this would be handled in `PlayerFactory` and `RegistryTransfer::xfer_inventory_entt`
class PlayerInventorySlot : public CarryItem
{
public:
  PlayerInventorySlot( Sprites::SpriteMetaType type )
      : CarryItem( type )
  {
  }
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_INVENTORY_CARRYITEM_HPP_