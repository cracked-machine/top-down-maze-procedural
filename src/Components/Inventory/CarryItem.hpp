#ifndef SRC_INVENTORY_CARRYITEM_HPP_
#define SRC_INVENTORY_CARRYITEM_HPP_

#include <Sprites/SpriteMetaType.hpp>
#include <Stats/BaseAction.hpp>
#include <utility>

namespace ProceduralMaze::Cmp
{

// Keeping the type generic allows easy qeury/view on all components in the game
class CarryItem
{
public:
  CarryItem( Sprites::SpriteMetaType type )
      : type( std::move( type ) )
  {
  }

  Sprites::SpriteMetaType type;

  //! @brief The action and its effects that can be applied to the player
  std::set<BaseAction> action_fx_list;
};

// Represents a single slot in the player inventory. If we want multiple slots we would need a "slot id",
// and this would be handled in `PlayerFactory` and `RegistryTransfer::xfer_inventory_entt`
class PlayerInventorySlot : public CarryItem
{
public:
  PlayerInventorySlot( Sprites::SpriteMetaType type )
      : CarryItem( std::move( type ) )
  {
  }
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_INVENTORY_CARRYITEM_HPP_