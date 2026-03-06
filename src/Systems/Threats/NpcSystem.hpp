#ifndef SRC_SYSTEMS_NPCSYSTEM_HPP__
#define SRC_SYSTEMS_NPCSYSTEM_HPP__

#include <Systems/BaseSystem.hpp>

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <entt/entity/fwd.hpp>

// clang-format off
namespace ProceduralMaze::Sprites { class SpriteFactory; }
namespace ProceduralMaze::Sys { class Store; }
namespace ProceduralMaze::Audio { class SoundBank; }
// clang-format on
namespace ProceduralMaze::Cmp
{
class Direction;
class LerpPosition;
class NpcShockwave;

} // namespace ProceduralMaze::Cmp

namespace ProceduralMaze::PathFinding
{
class SpatialHashGrid;
}

namespace ProceduralMaze::Sys
{

class NpcSystem : public BaseSystem
{
public:
  NpcSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief Update the NpcSystem
  //! @param dt Delta time since last update call
  void update( sf::Time dt, PathFinding::SpatialHashGrid *spatial_grid );

  //! @brief Checks if the Npc's movement to a given position is valid
  //! Validates whether the Npc can move to the specified position by checking
  //! for collisions with walls, boundaries, or other obstacles in the game world.
  //! @param player_position The target position to validate for Npc movement
  //! @return true if the movement is valid and allowed, false otherwise
  bool is_valid_move( const sf::FloatRect &player_position );

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

  void find_pushback_position( const Cmp::Direction &npc_direction );

  //! @brief Scan for players within NPC scan bounds
  //! @param player_entity The entity of the player
  void scan_player_distances( entt::entity player_entity );

  void update_animation();

  void update_shockwaves();
  sf::Clock shockwave_update_clock;

  void checkShockwaveObstacleCollision( entt::entity shockwave_entity, Cmp::NpcShockwave &shockwave );

  sf::Time m_scan_accumulator;
  sf::Time m_animation_accumulator;
  sf::Time m_bones_accumulator;

  PathFinding::SpatialHashGrid *m_spatial_grid{ nullptr };
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_NPCSYSTEM_HPP__
