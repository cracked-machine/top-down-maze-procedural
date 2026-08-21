#ifndef SRC_SYSTEMS_STORES_NPCSTORE_HPP__
#define SRC_SYSTEMS_STORES_NPCSTORE_HPP__

#include <Components/Npc/Npc.hpp>
#include <Sprites/SpriteMetaType.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/Stores/BaseStore.hpp>

namespace Game::Sys
{

//! @brief Singleton store of NPC metadata (Cmp::Npc::NPC, keyed by NPC id) loaded from res/json/npc.json.
class NpcStore : public StoreSingleton<NpcStore, Cmp::Npc::NPC>
{
public:
  //! @brief Construct a new Npc Store object
  //! @param reg
  //! @param window
  //! @param sprite_factory
  //! @param sound_bank
  NpcStore( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );
  ~NpcStore() {}

  //! @brief Populates m_store with Cmp::Npc::NPC components
  void init_store();
};

} // namespace Game::Sys

#endif // SRC_SYSTEMS_STORES_NPCSTORE_HPP__
