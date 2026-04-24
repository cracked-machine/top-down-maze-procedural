#ifndef SRC_SYSTEMS_ITEMSTORE_HPP_
#define SRC_SYSTEMS_ITEMSTORE_HPP_

#include <Sprites/SpriteMetaType.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/Stores/BaseStore.hpp>

namespace ProceduralMaze::Sys
{

class ItemStore : public StoreSingleton<ItemStore, Cmp::InventoryItem>
{
public:
  ItemStore( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );
  ~ItemStore() {}

  //! @brief Populates m_store with InventoryItem components
  void init_store();
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_ITEMSTORE_HPP_