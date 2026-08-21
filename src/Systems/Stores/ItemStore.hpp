#ifndef SRC_SYSTEMS_STORES_ITEMSTORE_HPP__
#define SRC_SYSTEMS_STORES_ITEMSTORE_HPP__

#include <Sprites/SpriteMetaType.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/Stores/BaseStore.hpp>

namespace Game::Sys
{

//! @brief Singleton store of item metadata (Cmp::WorldItem, keyed by item id) loaded from res/json/items.json.
class ItemStore : public StoreSingleton<ItemStore, Cmp::WorldItem>
{
public:
  //! @brief Construct a new Item Store object
  //! @param reg
  //! @param window
  //! @param sprite_factory
  //! @param sound_bank
  ItemStore( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );
  ~ItemStore() {}

  //! @brief Populates m_store with InventoryItem components
  void init_store();
};

} // namespace Game::Sys

#endif // SRC_SYSTEMS_STORES_ITEMSTORE_HPP__
