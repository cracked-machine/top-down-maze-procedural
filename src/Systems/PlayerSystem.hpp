#ifndef SRC_SYSTEMS_PLAYERSYSTEM_HPP__
#define SRC_SYSTEMS_PLAYERSYSTEM_HPP__

#include <Sprites/SpriteSheet.hpp>
#include <Systems/BaseSystem.hpp>

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <entt/entity/fwd.hpp>

// clang-format off
namespace Game::Events { class DropInventoryEvent; class PlayerActionEvent; class PlayerMortalityEvent; }
namespace Game::Sprites { class SpriteSheet; }
namespace Game::Cmp { class Direction; class LerpPosition; class NpcShockwave; class Position; class PlayerMortality; class AnimData; }
namespace Game::Cmp::Peristent { class EffectsVolume; } 
namespace Game::PathFinding { class SpatialHashGrid; }
// clang-format on

namespace Game::Sys
{

class PlayerSystem : public BaseSystem
{
public:
  enum class FootStepSfx { NONE, GRAVEL, FLOORBOARDS };
  PlayerSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
                entt::dispatcher &scenemanager_event_dispatcher );

  //! @brief init the weak pointer for the pathfinding navmesh
  //! @param npc_navmesh
  //! @param open_navmesh
  void init( const PathFinding::SpatialHashGridSharedPtr &npc_navmesh, const PathFinding::SpatialHashGridSharedPtr &player_navmesh,
             const PathFinding::SpatialHashGridSharedPtr &open_navmesh )
  {
    m_npc_navmesh = npc_navmesh;
    m_player_navmesh = player_navmesh;
    m_open_navmesh = open_navmesh;
  }

  //! @brief Update the player system.
  void update( sf::Time dt, FootStepSfx footstep_sfx = FootStepSfx::GRAVEL );

  //! @brief
  //! @param type
  void play_footsteps_sound( FootStepSfx type );

  //! @brief
  void stop_footsteps_sound();

  //! @brief
  void disable_damage_cooldown();

  //! @brief
  void enable_damage_cooldown();

  //! @brief Force the player's very next damage check to land regardless of the damage cooldown timer.
  void force_expire_damage_cooldown();

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

private:
  //! @brief
  //! @param dt
  //! @param collision_disabled
  void update_player_position( sf::Time dt );

  //! @brief Change to animation spritesheet for the players current direction.
  void update_player_animation();

  //! @brief Check if the player is dead
  //! @note This checks if the Cmp::PlayerMortality == State::DEAD, not check Cmp:PlayerStats (that is other system responsibility).
  void check_player_mortality();

  //! @brief Update InventoryItem/NPC action effects on player for Tick::SLOW.
  //! @note Cmp::CollisionAction and Cmp::ProjectileAction are excluded. See NpcSystem.
  //! @param dt
  void check_timed_action_side_effects( sf::Time dt );

  //! @brief Update the clocks for the timed actions
  //! @param dt
  void update_timed_action_clocks( sf::Time dt );

  //! @brief Send mortality event if player fear/despair is 100%
  void check_player_max_fear_despair();

  //! @brief If player is carry suitable weapon did the action event occur in NPC vicinity?
  void check_player_axe_npc_kill();

  void move_obstacle( const sf::FloatRect &target_position );
  void check_player_can_push( sf::Time dt );
  void check_player_can_pull( sf::Time dt );

  //! @brief Fade the player alpha if they intiiated wormhole jump.
  void fade_player_on_wormhole_jump();

  //! @brief Blink the player if damage cooldown was activated.
  void blink_player();

  //! @brief Remove the CarryItem from player inventory and place it into the world
  //! @param reg the ECS registry
  //! @param pos the postion to place the item
  //! @param sprite the spritesheet object
  //! @param inventory_slot_cmp_entt the player inventory slot entt
  //! @return entt::entity
  void drop_inventory_slot_into_world( sf::Vector2f pos, entt::entity inventory_slot_entt );

  //! @brief
  //! @param reg
  //! @param world_item_entt
  void pickup_world_item( entt::registry &reg, entt::entity world_item_entt );

  //! @brief Checks if the player's movement to a given position is valid
  //! Validates whether the player can move to the specified position by checking
  //! for collisions with walls, boundaries, or other obstacles in the game world.
  //! @param player_position The target position to validate for player movement
  //! @return true if the movement is valid and allowed, false otherwise
  bool is_valid_move( const sf::FloatRect &target_position );

  //! @brief Single drop, no pickup
  //! @param ev
  void on_drop_inventory_event( Game::Events::DropInventoryEvent ev );

  //! @brief Attack, Dig and Drop/Pickup events
  //! @param ev
  void on_player_action_event( Game::Events::PlayerActionEvent ev );

  //! @brief Checking if Cmp::PlayerNoPath should be activated.
  //! @param dt
  void update_player_no_path_cmp( sf::Time dt );

  //! @brief Various ways in which the player can die. Starts the death animation loop.
  //! @param ev
  void on_player_mortality_event( Game::Events::PlayerMortalityEvent ev );

  //! @brief Use this to send events to the scene manager
  entt::dispatcher &m_scenemanager_event_dispatcher;

  //! @brief Synchronizes the action stat modifiers.
  sf::Time m_timed_action_sync_clock;

  //! @brief Global fear stat increase timer when player is standing in darkness.
  sf::Time m_darkness_fear_clock;

  //! @brief Prevent player from spamming the drop inventory action.
  sf::Clock m_inventory_cooldown_timer;

  //! @brief Cooldown for checking if Cmp::PlayerNoPath should be activated.
  sf::Time m_plantcheck_accumulator;

  //! @brief Weak pointer to the pathfinding navmesh.
  PathFinding::SpatialHashGridWeakPtr m_npc_navmesh;
  PathFinding::SpatialHashGridWeakPtr m_player_navmesh;
  PathFinding::SpatialHashGridWeakPtr m_open_navmesh;

  //! @brief Prevent the player from moving when running
  sf::Clock m_movement_suppress_clock{};

  //! @brief Tracks how long the bow has been held drawn, between DRAW_BOW and RELEASE_BOW.
  sf::Clock m_bow_draw_clock;
  //! @brief True while the bow is being drawn (left mouse button held with bow equipped).
  bool m_bow_drawing{ false };
};

} // namespace Game::Sys

#endif // SRC_SYSTEMS_PLAYERSYSTEM_HPP__
