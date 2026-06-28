#ifndef SRC_COMPONENTS_INVENTORY_WORLDITEM_HPP__
#define SRC_COMPONENTS_INVENTORY_WORLDITEM_HPP__

#include <Components/Stats/BaseAction.hpp>
#include <Sprites/SpriteMetaType.hpp>

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
  WorldItem( std::string item_type, Sprites::SpriteMetaType sprite_type )
      : item_type( std::move( item_type ) ),
        sprite_type( std::move( sprite_type ) )
  {
  }

  std::string item_type;

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

#endif // SRC_COMPONENTS_INVENTORY_WORLDITEM_HPP__
