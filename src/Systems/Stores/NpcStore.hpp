#ifndef SRC_SYSTEMS_NPCSTORE_HPP_
#define SRC_SYSTEMS_NPCSTORE_HPP_

#include <Components/Npc/Npc.hpp>
#include <Sprites/SpriteMetaType.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/Stores/BaseStore.hpp>

namespace ProceduralMaze::Sys
{

class NpcStore : public StoreSingleton<NpcStore, Cmp::NPC>
{
public:
  NpcStore( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );
  ~NpcStore() {}

  //! @brief Populates m_store with Cmp::NPC components
  void init_store();
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_NPCSTORE_HPP_