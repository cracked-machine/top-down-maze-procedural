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

  //! @brief init the weak pointer for the pathfinding navmesh
  //! @param pathfinding_navmesh
  void init( const PathFinding::SpatialHashGridSharedPtr &pathfinding_navmesh ) { m_pathfinding_navmesh = pathfinding_navmesh; }

  //! @brief Update the NpcSystem
  //! @param dt Delta time since last update call
  void update( sf::Time dt );

  //! @brief Checks if the Npc's movement to a given position is valid
  //! Validates whether the Npc can move to the specified position by checking
  //! for collisions with walls, boundaries, or other obstacles in the game world.
  //! @param player_position The target position to validate for Npc movement
  //! @return true if the movement is valid and allowed, false otherwise
  bool is_valid_move( const sf::FloatRect &player_position );

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

private:
  // Updates lerp movement for NPCs
  void update_movement( sf::Time globalDeltaTime );

  // Check for player collision proximity with NPC containers
  void check_bones_reanimation();

  //! @brief Check Player/NPC collision for Cmp::CollisionActions::Tick::ONCE
  //! @note Player cooldown and knockback are ENABLED.
  void check_once_collision();

  //! @brief Check Player/NPC collision for Cmp::CollisionActions::Tick::SLOW
  //! @note Player cooldown and knockback are DISABLED.
  void check_slow_collision( sf::Time dt );

  //! @brief Check Player/NPC collision for Cmp::CollisionActions::Tick::FAST
  //! @note Player cooldown and knockback are DISABLED.
  void check_fast_collision( sf::Time dt );

  void find_pushback_position( const Cmp::Direction &npc_direction );

  void update_pathfinding( entt::entity player_entity );

  void update_animation();

  void update_shockwaves();
  sf::Clock shockwave_update_clock;

  void checkShockwaveObstacleCollision( entt::entity shockwave_entity, Cmp::NpcShockwave &shockwave );

  sf::Time m_scan_accumulator;
  sf::Time m_animation_accumulator;
  sf::Time m_bones_accumulator;

  PathFinding::SpatialHashGridWeakPtr m_pathfinding_navmesh;

  sf::Time m_slow_tick_action_effects_time;
  sf::Time m_fast_tick_action_effects_time;
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_NPCSYSTEM_HPP__
