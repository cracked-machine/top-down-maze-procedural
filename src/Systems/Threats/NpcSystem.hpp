#ifndef SRC_SYSTEMS_NPCSYSTEM_HPP__
#define SRC_SYSTEMS_NPCSYSTEM_HPP__

#include <Components/Direction.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/Npc/NpcShockwave.hpp>
#include <Systems/BaseSystem.hpp>

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>

#include <spdlog/spdlog.h>

// clang-format off
namespace ProceduralMaze::Sprites { class SpriteFactory; }
namespace ProceduralMaze::Sys { class SystemStore; }
namespace ProceduralMaze::Audio { class SoundBank; }
// clang-format on

namespace ProceduralMaze::Sys
{

class NpcSystem : public BaseSystem
{
public:
  NpcSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief Update the NpcSystem
  //! @param dt Delta time since last update call
  void update( sf::Time dt );

  //! @brief Check if diagonal movement should be blocked due to adjacent obstacles
  bool isDiagonalBlocked( const sf::FloatRect &current_pos, const sf::Vector2f &diagonal_direction );

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

private:
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

  void emit_shockwave( entt::entity npc_entt );
  void update_shockwaves();
  sf::Clock shockwave_update_clock;

  void checkShockwaveObstacleCollision( entt::entity shockwave_entity, Cmp::NpcShockwave &shockwave );
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_NPCSYSTEM_HPP__
