#ifndef SRC_INVENTORY_CARRYITEM_HPP_
#define SRC_INVENTORY_CARRYITEM_HPP_

#include <Sprites/SpriteMetaType.hpp>
#include <Stats/BaseAction.hpp>

#include <typeindex>
#include <unordered_map>
#include <utility>

namespace ProceduralMaze::Cmp
{

// Keeping the type generic allows easy qeury/view on all components in the game
class InventoryItem
{
public:
  InventoryItem() = default;
  InventoryItem( Sprites::SpriteMetaType type )
      : type( std::move( type ) )
  {
  }

  Sprites::SpriteMetaType type;

  //! @brief The action and its effects that can be applied to the player
  std::unordered_map<std::type_index, BaseAction> action_fx_map;
};

// Represents a single slot in the player inventory. If we want multiple slots we would need a "slot id",
// and this would be handled in `PlayerFactory` and `RegistryTransfer::xfer_inventory_entt`
struct PlayerInventorySlot
{
  InventoryItem m_item;
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_INVENTORY_CARRYITEM_HPP_