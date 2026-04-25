#ifndef SRC_INVENTORY_CARRYITEM_HPP_
#define SRC_INVENTORY_CARRYITEM_HPP_

#include <Sprites/SpriteMetaType.hpp>
#include <Stats/BaseAction.hpp>

#include <typeindex>
#include <unordered_map>
#include <utility>

namespace ProceduralMaze::Cmp
{

//! @brief An item found in the world or placed in a players inventory slot.
//         This defines both the sprite type and the action effects associated with the item.
class InventoryItem
{
public:
  InventoryItem() = default;
  InventoryItem( Sprites::SpriteMetaType sprite_type )
      : sprite_type( std::move( sprite_type ) )
  {
  }

  //! @brief The associated sprite
  Sprites::SpriteMetaType sprite_type;

  //! @brief The action and its effects that can be applied to the player
  std::unordered_map<std::type_index, BaseAction> actions;
};

// Represents a single slot in the player inventory. If we want multiple slots we would need a "slot id",
// and this would be handled in `PlayerFactory` and `RegistryTransfer::xfer_inventory_entt`
struct PlayerInventorySlot
{
  InventoryItem m_item;
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_INVENTORY_CARRYITEM_HPP_