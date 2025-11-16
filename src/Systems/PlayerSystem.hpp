#ifndef __CMP_PLAYERSYSTEM_HPP__
#define __CMP_PLAYERSYSTEM_HPP__

#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/PlayerMortality.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <entt/entity/registry.hpp>

#include <Components/Direction.hpp>
#include <Components/PCDetectionBounds.hpp>
#include <Components/Persistent/BlastRadius.hpp>
#include <Components/Persistent/BombInventory.hpp>
#include <Components/Persistent/PlayerDetectionScale.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Position.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sys
{

class PlayerSystem : public BaseSystem
{
public:
  PlayerSystem( sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  /**
   * @brief Adds a new player entity to the game world.
   *
   * Creates and initializes a player entity with default components for physics processing
   * and any other player-specific attributes.
   * This method is typically called during game initialization.
   */
  void add_player_entity();

  [[nodiscard]] Cmp::PlayerMortality::State check_player_mortality();

  /**
   * @brief Updates the player's movement based on input and physics
   *
   * This function handles the player's movement logic, including position updates,
   * velocity calculations, and collision detection. It processes user input and
   * applies movement transformations over the given time delta.
   *
   * @param globalDeltaTime The time elapsed since the last frame update, used for
   *                  frame-rate independent movement calculations
   * @param skip_collision_check Optional parameter to bypass collision detection.
   *                            Defaults to false. When true, the player can move
   *                            through walls and other collision objects
   */
  void update_movement( sf::Time globalDeltaTime, bool skip_collision_check = false );

  void play_footsteps_sound();
  void stop_footsteps_sound();

private:
};

} // namespace ProceduralMaze::Sys

#endif // __CMP_PLAYERSYSTEM_HPP__
