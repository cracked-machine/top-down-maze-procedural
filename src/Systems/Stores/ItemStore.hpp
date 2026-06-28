#ifndef SRC_SYSTEMS_STORES_ITEMSTORE_HPP__
#define SRC_SYSTEMS_STORES_ITEMSTORE_HPP__

#include <Sprites/SpriteMetaType.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/Stores/BaseStore.hpp>

namespace Game::Sys
{

class ItemStore : public StoreSingleton<ItemStore, Cmp::WorldItem>
{
public:
  ItemStore( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );
  ~ItemStore() {}

  //! @brief Populates m_store with InventoryItem components
  void init_store();
};

} // namespace Game::Sys

#endif // SRC_SYSTEMS_STORES_ITEMSTORE_HPP__
