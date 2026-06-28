#ifndef SRC_SYSTEMS_ActionSystem_HPP__
#define SRC_SYSTEMS_ActionSystem_HPP__

#include <Components/Persistent/EffectsVolume.hpp>
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

// ActionSystem handles player actions within the game. This system is mainly event-driven, responding to Events::PlayerActionEvent.
class ActionSystem : public BaseSystem
{
public:
  ActionSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief init the weak pointer for the pathfinding navmesh
  //! @param npc_navmesh
  void init( const PathFinding::SpatialHashGridSharedPtr &npc_navmesh, const PathFinding::SpatialHashGridSharedPtr &player_navmesh )
  {
    m_npc_navmesh = npc_navmesh;
    m_player_navmesh = player_navmesh;
  }

  // void load_sounds();
  // additional updates via the main game loop
  void update( sf::Time dt );

  // Event handler for player actions
  void on_player_action( const Events::PlayerActionEvent &event );

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

private:
  /**
   * @brief Checks for collisions between the player's digging action and obstacles in the maze.
   *
   * This function detects when the player attempts to dig into or through obstacles,
   * such as walls or other impassable terrain elements. It handles the collision
   * detection logic to determine if a dig action should be allowed or blocked.
   *
   * @note This function is not called via the main game loop, but rather in response to
   * Events::PlayerActionEvent::DIG (see on_player_action).
   */
  void check_player_dig_obstacle_collision();
  void check_player_dig_plant_collision();
  void check_player_smash_pot();
  void select_moveable_obstacle();
  void deselect_all_moveable_obstacles();

  sf::Time m_plantcheck_accumulator;

  // Cooldown clock to manage digging intervals
  sf::Clock m_dig_cooldown_clock;

  //! @brief Structure to hold pickaxe sound information
  //! Used to manage multiple pickaxe sound effects
  struct PickAxeSound
  {
    std::filesystem::path path;
    sf::SoundBuffer buffer;
  };

  PathFinding::SpatialHashGridWeakPtr m_npc_navmesh;
  PathFinding::SpatialHashGridWeakPtr m_player_navmesh;
};
} // namespace Game::Sys

#endif // SRC_SYSTEMS_ActionSystem_HPP__