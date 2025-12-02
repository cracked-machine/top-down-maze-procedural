#ifndef SRC_SYSTEMS_NPCSYSTEM_HPP__
#define SRC_SYSTEMS_NPCSYSTEM_HPP__

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>

#include <spdlog/spdlog.h>

#include <Components/Direction.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/NPC.hpp>
#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/Persistent/NpcActivateScale.hpp>
#include <Components/Persistent/NpcScanScale.hpp>
#include <Components/Position.hpp>
#include <Events/NpcCreationEvent.hpp>
#include <Events/NpcDeathEvent.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/BaseSystem.hpp>

#include <queue>

namespace ProceduralMaze::Sys
{

using PlayerDistanceQueue = std::priority_queue<std::pair<int, entt::entity>, std::vector<std::pair<int, entt::entity>>,
                                                std::greater<std::pair<int, entt::entity>>>;

class NpcSystem : public BaseSystem
{
public:
  NpcSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief Update the NpcSystem
  //! @param dt Delta time since last update call
  void update( sf::Time dt );

  //! @brief Check if diagonal movement should be blocked due to adjacent obstacles
  bool isDiagonalBlocked( const sf::FloatRect &current_pos, const sf::Vector2f &diagonal_direction );

  // Event handler for remove_npc_entity()
  void on_npc_death( const Events::NpcDeathEvent &event );

  // Event handler for add_npc_entity()
  void on_npc_creation( const Events::NpcCreationEvent &event );

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

private:
  // Converts a NpcContainer entity into an active NPC entity. Called by event: NpcCreationEvent
  void add_npc_entity( const Events::NpcCreationEvent &event );

  // Removes an active NPC entity from the game. Called by event: NpcDeathEvent
  void remove_npc_entity( entt::entity npc_entity );

  // Updates lerp movement for NPCs
  void update_movement( sf::Time globalDeltaTime );

  // Check for player collision proximity with NPC containers
  void check_bones_reanimation();

  // Check for player collision with NPCs
  void check_player_to_npc_collision();

  //! @brief Check for player collision with NPCs
  //! @param player_pos The position of the player
  //! @param npc_pos The position of the NPC
  //! @param player_direction The direction the player is facing
  //! @param pushback_distance The distance to push the player back
  //! @return sf::Vector2f
  sf::Vector2f findValidPushbackPosition( const sf::Vector2f &player_pos, const sf::Vector2f &npc_pos, const sf::Vector2f &player_direction,
                                          float pushback_distance );

  //! @brief Scan for players within NPC scan bounds
  //! @param player_entity The entity of the player
  void scanForPlayers( entt::entity player_entity );

  //! @brief Add candidate lerp position and direction to NPC
  //! @param npc_entity The entity of the NPC
  //! @param candidate_dir The candidate direction to move
  //! @param candidate_lerp_pos The candidate lerp position to move to
  void add_candidate_lerp( entt::entity npc_entity, Cmp::Direction candidate_dir, Cmp::LerpPosition candidate_lerp_pos );
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_NPCSYSTEM_HPP__
