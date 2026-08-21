#ifndef SRC_SYSTEMS_ACTIONSYSTEM_HPP__
#define SRC_SYSTEMS_ACTIONSYSTEM_HPP__

#include <Components/Persistent/EffectsVolume.hpp>
#include <Events/DropInventoryEvent.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <PathFinding/SpatialHashGrid.hpp>
#include <Systems/BaseSystem.hpp>

#include <SFML/Audio/AudioResource.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/System/Clock.hpp>
#include <filesystem>

namespace Game::PathFinding
{
class SpatialHashGrid;
}

namespace Game::Sys
{

//! @brief Handles player actions within the game. This system is mainly event-driven, responding to
//! Events::PlayerActionEvent.
class ActionSystem : public BaseSystem
{
public:
  //! @brief Construct a new Action System object
  //! @param reg
  //! @param window
  //! @param sprite_factory
  //! @param sound_bank
  ActionSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief init the weak pointer for the pathfinding navmesh
  //! @param npc_navmesh
  void init( const PathFinding::SpatialHashGridSharedPtr &npc_navmesh, const PathFinding::SpatialHashGridSharedPtr &player_navmesh,
             const PathFinding::SpatialHashGridSharedPtr &ghost_navmesh = nullptr )
  {
    m_npc_navmesh = npc_navmesh;
    m_player_navmesh = player_navmesh;
    m_ghost_navmesh = ghost_navmesh;
  }

  //! @brief frame updates
  //! @param dt
  void update( sf::Time dt );

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

private:
  //! @brief Event handler for player actions
  //! @param event
  void on_player_action( const Events::PlayerActionEvent &event );

  //! @brief Single drop, no pickup
  //! @param ev
  void on_drop_inventory_event( Game::Events::DropInventoryEvent ev );

  //! @brief Uses Cmp::SelectedPosition to mark the targetted Cmp::Obstacle then applies digging damage until the obstacle is destroyed.
  void check_player_dig_obstacle_collision();

  //! @brief Uses Cmp::SelectedPosition to mark the targetted Cmp::PlantMultiBlock then applies digging damage until the obstacle is destroyed.
  void check_player_dig_plant_collision();

  //! @brief Replace the targetted Cmp::LootContainer with loot item via Events::CreateItemEvent (see ItemSystem::on_create_item_event)
  void check_player_smash_pot();

  //! @brief If player is carry suitable weapon did the action event occur in NPC vicinity?
  void check_player_axe_npc_kill();

  //! @brief Remove the CarryItem from player inventory and place it into the world
  //! @param reg the ECS registry
  //! @param pos the postion to place the item
  //! @param sprite the spritesheet object
  //! @param inventory_slot_cmp_entt the player inventory slot entt
  //! @return entt::entity
  void drop_inventory_item( sf::Vector2f pos, entt::entity inventory_slot_entt );

  //! @brief Add the world item to the player's inventory and destroy the world entity.
  //! @param reg
  //! @param world_item_entt
  void pickup_world_item( entt::registry &reg, entt::entity world_item_entt );

  //! @brief Uses Cmp::SelectedPosition to mark the targetted Cmp::Obstacle/Cmp::Moveable entity.
  void select_moveable_obstacle();

  //! @brief Removes all Cmp::SelectedPosition components
  void reset_all_selected_positions();

  //! @brief True if the player's last dig action is still within its cooldown window
  bool is_digging_on_cooldown();

  //! @brief all grid positions that block NPC pathfinding
  PathFinding::SpatialHashGridWeakPtr m_npc_navmesh;

  //! @brief All grid positions that block ghost NPC pathfinding
  PathFinding::SpatialHashGridWeakPtr m_ghost_navmesh;

  //! @brief All grid positions that block player movement
  PathFinding::SpatialHashGridWeakPtr m_player_navmesh;

  //! @brief Prevent player from spamming the drop inventory action.
  sf::Clock m_inventory_cooldown_timer;
};
} // namespace Game::Sys

#endif // SRC_SYSTEMS_ACTIONSYSTEM_HPP__
