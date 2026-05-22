#ifndef SRC_INVENTORY_CARRYITEM_HPP_
#define SRC_INVENTORY_CARRYITEM_HPP_

#include <Sprites/SpriteMetaType.hpp>
#include <Stats/BaseAction.hpp>

#include <typeindex>
#include <unordered_map>
#include <utility>

namespace Game::Cmp
{

//! @brief An item found in the world or placed in a players inventory slot.
//         This defines both the sprite type and the action effects associated with the item.
class WorldItem
{
public:
  WorldItem() = default;
  WorldItem( Sprites::SpriteMetaType sprite_type )
      : sprite_type( std::move( sprite_type ) )
  {
  }

  //! @brief The associated sprite
  Sprites::SpriteMetaType sprite_type;

  struct ActionTimePair
  {
    BaseAction action;
    sf::Time time;
  };

  //! @brief The action and its effects that can be applied to the player
  std::unordered_map<std::type_index, ActionTimePair> actions;
};

} // namespace Game::Cmp

#endif // SRC_INVENTORY_CARRYITEM_HPP_