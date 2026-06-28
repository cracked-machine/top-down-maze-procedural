#ifndef SRC_SYSTEMS_THREATS_NPCSYSTEM_HPP__
#define SRC_SYSTEMS_THREATS_NPCSYSTEM_HPP__

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
  //! @brief Construct a new Npc System object
  //! @param reg
  //! @param window
  //! @param sprite_factory
  //! @param sound_bank
  NpcSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief init the weak pointer for the pathfinding navmesh
  //! @param npc_navmesh
  //! @param open_navmesh
  void init( const PathFinding::SpatialHashGridSharedPtr &npc_navmesh, const PathFinding::SpatialHashGridSharedPtr &open_navmesh )
  {
    m_npc_navmesh = npc_navmesh;
    m_open_navmesh = open_navmesh;
  }

  //! @brief Update the NpcSystem
  //! @param dt Delta time since last update call
  void update( sf::Time dt );

  //! @brief Checks if the Npc's movement to a given position is valid
  //! Validates whether the Npc can move to the specified position by checking
  //! for collisions with walls, boundaries, or other obstacles in the game world.
  //! @param target_position The target position to validate for Npc movement
  //! @return true if the movement is valid and allowed, false otherwise
  bool is_valid_move( const sf::FloatRect &target_position );

  //! @brief Create a wisp NPC at a random location. Uses `m_open_navmesh` to pathfind freely around the scene.
  void spawn_wisp();

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

private:
  //! @brief Updates lerp movement for all NPCs
  //! @param dt
  void update_movement( sf::Time dt );

  //! @brief Update lerp movement for specific NPC entity and navmesh
  //! @param navmesh
  //! @param npc_entity
  //! @param dt
  void update_movement_for( PathFinding::SpatialHashGrid &navmesh, entt::entity npc_entity, sf::Time dt );

  //! @brief Update the NPC sfx
  void update_sfx();

  //! @brief Check for player collision proximity with NPC containers
  void check_npc_container_collision();

  //! @brief Add a new pathfinding target for the wisp NPC.
  //! @param wisp_entt
  void reset_wisp_target( entt::entity wisp_entt );

  //! @brief Check Player/NPC collision for Cmp::CollisionActions with zero tick.
  //! @note Player cooldown and knockback are ENABLED.
  void check_once_collision();

  //! @brief Check Player/NPC collision for Cmp::CollisionActions with non-zero tick.
  //! @note Player cooldown and knockback are DISABLED.
  void check_timed_collision( sf::Time dt );

  //! @brief Find a suitable knockback position for the player
  //! @param npc_direction
  void find_pushback_position( const Cmp::Direction &npc_direction );

  //! @brief Update pathfinding for all NPCs.
  //! @param dt
  void update_pathfinding( sf::Time dt );

  //! @brief Update pathfinding for specific NPC entities and navmesh.
  //! @param navmesh
  //! @param target_pos
  //! @param npc_entity
  //! @param target_in_spawn
  void update_pathfinding_for( PathFinding::SpatialHashGrid &navmesh, const Cmp::Position &target_pos, entt::entity npc_entity,
                               bool target_in_spawn );

  //! @brief Update NPC animation
  void update_animation();

  //! @brief track time since last pathfinding update
  sf::Time m_pathfinding_timer;

  //! @brief Track time since last animation frame
  sf::Time m_anim_timer;

  //! @brief Track time since has container activiation check
  sf::Time m_container_timer;

  //! @brief Delay the target reset once the pathfinding has comepleted.
  sf::Clock m_wisp_target_reset_clock;

  //! @brief Used for NPCs to pathfind around blocking obstacles
  PathFinding::SpatialHashGridWeakPtr m_npc_navmesh;

  //! @brief Used for NPCs to move freely within the level boundaries.
  PathFinding::SpatialHashGridWeakPtr m_open_navmesh;
};

} // namespace Game::Sys

#endif // SRC_SYSTEMS_THREATS_NPCSYSTEM_HPP__
