#ifndef __CMP_PLAYERSYSTEM_HPP__
#define __CMP_PLAYERSYSTEM_HPP__

#include <Components/Movement.hpp>
#include <Components/Position.hpp>
#include <Direction.hpp>
#include <PCDetectionBounds.hpp>
#include <Persistent/BlastRadius.hpp>
#include <Persistent/BombInventory.hpp>
#include <Persistent/LandAcceleration.hpp>
#include <Persistent/LandDeacceleration.hpp>
#include <Persistent/LandMaxSpeed.hpp>
#include <Persistent/PCDetectionScale.hpp>
#include <Persistent/WaterAcceleration.hpp>
#include <Persistent/WaterDeceleration.hpp>
#include <Persistent/WaterMaxSpeed.hpp>
#include <PlayableCharacter.hpp>
#include <Systems/BaseSystem.hpp>
#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Sys {

class PlayerSystem : public BaseSystem
{
public:
  PlayerSystem( ProceduralMaze::SharedEnttRegistry registry )
      : BaseSystem( registry )
  {
  }

  /**
   * @brief Initializes the Components as context variables. These components are not owned by any
   * entity.
   * @details Call this before using any PlayerSystem functionality that relies on an initialized
   * context.
   * @note Intended to be invoked once during system setup or scene load.
   */
  void init_context();

  /**
   * @brief Adds a new player entity to the game world.
   *
   * Creates and initializes a player entity with default components for physics processing
   * and any other player-specific attributes.
   * This method is typically called during game initialization.
   */
  void add_player_entity();

  /**
   * @brief Updates the player's movement based on input and physics
   *
   * This function handles the player's movement logic, including position updates,
   * velocity calculations, and collision detection. It processes user input and
   * applies movement transformations over the given time delta.
   *
   * @param deltaTime The time elapsed since the last frame update, used for
   *                  frame-rate independent movement calculations
   * @param skip_collision_check Optional parameter to bypass collision detection.
   *                            Defaults to false. When true, the player can move
   *                            through walls and other collision objects
   */
  void update_movement( sf::Time deltaTime, bool skip_collision_check = false );

  /**
   * @brief Updates the player's animation based on the elapsed time.
   *
   * This function handles the progression of player animation frames,
   * updating sprite states, animation timers, and visual effects based
   * on the player's current state and actions.
   *
   * @param deltaTime The time elapsed since the last frame update
   */
  void update_player_animation( sf::Time deltaTime );
};

} // namespace ProceduralMaze::Sys

#endif // __CMP_PLAYERSYSTEM_HPP__
