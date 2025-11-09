#ifndef __SYS_NPCSYSTEM_HPP__
#define __SYS_NPCSYSTEM_HPP__

#include <Components/Persistent/EffectsVolume.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>

#include <spdlog/spdlog.h>

#include <Components/Direction.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/NPC.hpp>
#include <Components/NPCScanBounds.hpp>
#include <Components/Persistent/NpcActivateScale.hpp>
#include <Components/Persistent/NpcScanScale.hpp>
#include <Components/Position.hpp>
#include <Events/NpcCreationEvent.hpp>
#include <Events/NpcDeathEvent.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sys {

class NpcSystem : public BaseSystem
{
public:
  NpcSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
             Audio::SoundBank &sound_bank );

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  // Converts a NpcContainer entity into an active NPC entity. Called by event: NpcCreationEvent
  void add_npc_entity( const Events::NpcCreationEvent &event );

  // Removes an active NPC entity from the game. Called by event: NpcDeathEvent
  void remove_npc_entity( entt::entity npc_entity );

  // Smoothly interpolates the position of NPCs. Called in the main update loop.
  void update_movement( sf::Time globalDeltaTime );

  // Check for player collision with bones obstacles to reanimate NPCs
  void check_bones_reanimation();

  // Check for player collision with NPCs
  void check_player_to_npc_collision();

  // Event handler for remove_npc_entity()
  void on_npc_death( const Events::NpcDeathEvent &event );

  // Event handler for add_npc_entity()
  void on_npc_creation( const Events::NpcCreationEvent &event );

private:
  sf::Vector2f findValidPushbackPosition( const sf::Vector2f &player_pos, const sf::Vector2f &npc_pos,
                                          const sf::Vector2f &player_direction, float pushback_distance );
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_NPCSYSTEM_HPP__
