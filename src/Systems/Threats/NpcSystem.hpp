#ifndef SRC_SYSTEMS_NPCSYSTEM_HPP__
#define SRC_SYSTEMS_NPCSYSTEM_HPP__

#include <Systems/BaseSystem.hpp>

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <entt/entity/fwd.hpp>

// clang-format off
namespace Game::Sprites { class SpriteFactory; }
namespace Game::Sys { class Store; }
namespace Game::Audio { class SoundBank; }
namespace Game::Cmp { class Position; }

// clang-format on
namespace Game::Cmp
{
class Direction;
class LerpPosition;
class NpcShockwave;

} // namespace Game::Cmp

namespace Game::PathFinding
{
class SpatialHashGrid;
}

namespace Game::Sys
{

class NpcSystem : public BaseSystem
{
public:
  NpcSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief init the weak pointer for the pathfinding navmesh
  //! @param pathfinding_navmesh
  //! @param open_navmesh
  void init( const PathFinding::SpatialHashGridSharedPtr &pathfinding_navmesh, const PathFinding::SpatialHashGridSharedPtr &open_navmesh )
  {
    m_pathfinding_navmesh = pathfinding_navmesh;
    m_open_navmesh = open_navmesh;
  }

  //! @brief Update the NpcSystem
  //! @param dt Delta time since last update call
  void update( sf::Time dt );

  //! @brief Checks if the Npc's movement to a given position is valid
  //! Validates whether the Npc can move to the specified position by checking
  //! for collisions with walls, boundaries, or other obstacles in the game world.
  //! @param player_position The target position to validate for Npc movement
  //! @return true if the movement is valid and allowed, false otherwise
  bool is_valid_move( const sf::FloatRect &target_position );

  void spawn_wisp();

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

private:
  // Updates lerp movement for NPCs
  void update_movement( sf::Time dt );
  void update_movement_for( PathFinding::SpatialHashGrid &navmesh, entt::entity npc_entity, sf::Time dt );

  void update_sfx();

  // Check for player collision proximity with NPC containers
  void check_bones_reanimation();

  void reset_wisp_target( entt::entity wisp_entt );

  //! @brief Check Player/NPC collision for Cmp::CollisionActions::Tick::ONCE
  //! @note Player cooldown and knockback are ENABLED.
  void check_once_collision();

  //! @brief Check Player/NPC collision for Cmp::CollisionActions::Tick::SLOW
  //! @note Player cooldown and knockback are DISABLED.
  void check_timed_collision( sf::Time dt );

  void find_pushback_position( const Cmp::Direction &npc_direction );

  void update_pathfinding( sf::Time dt );
  void update_pathfinding_for( PathFinding::SpatialHashGrid &navmesh, const Cmp::Position &target_pos, entt::entity npc_entity );

  void update_animation();

  void update_shockwaves();
  sf::Clock shockwave_update_clock;

  void checkShockwaveObstacleCollision( entt::entity shockwave_entity, Cmp::NpcShockwave &shockwave );

  sf::Time m_scan_accumulator;
  sf::Time m_animation_accumulator;
  sf::Time m_bones_accumulator;

  //! @brief Delay the target reset once the pathfinding has comepleted.
  sf::Clock m_wisp_target_reset_clock;

  PathFinding::SpatialHashGridWeakPtr m_pathfinding_navmesh;
  PathFinding::SpatialHashGridWeakPtr m_open_navmesh;
};

} // namespace Game::Sys

#endif // SRC_SYSTEMS_NPCSYSTEM_HPP__
