#ifndef __CMP_PLAYERSYSTEM_HPP__
#define __CMP_PLAYERSYSTEM_HPP__

#include <Components/Movement.hpp>
#include <Components/Position.hpp>
#include <Direction.hpp>
#include <PCDetectionBounds.hpp>
#include <Persistent/BlastRadius.hpp>
#include <Persistent/BombInventory.hpp>
#include <Persistent/FrictionCoefficient.hpp>
#include <Persistent/FrictionFalloff.hpp>
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
  PlayerSystem( std::shared_ptr<entt::basic_registry<entt::entity>> registry ) : BaseSystem( registry ) {}

  /**
   * @brief Initializes the Components as context variables. These components are not owned by any entity.
   * @details Call this before using any PlayerSystem functionality that relies on an initialized context.
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
   * @brief Updates the player system for the current frame
   *
   * This method is called once per frame to update the player's state,
   * movement calculations, and any other player-related logic that needs to be updated over time.
   *
   * @param deltaTime The time elapsed since the last frame update,
   *                  used for frame-rate independent calculations
   */
  void update( sf::Time deltaTime );
};

} // namespace ProceduralMaze::Sys

#endif // __CMP_PLAYERSYSTEM_HPP__
