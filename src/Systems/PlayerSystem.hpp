#ifndef __CMP_PLAYERSYSTEM_HPP__
#define __CMP_PLAYERSYSTEM_HPP__

#include <Components/Persistent/EffectsVolume.hpp>
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

namespace ProceduralMaze::Sys {

class PlayerSystem : public BaseSystem
{
public:
  PlayerSystem( ProceduralMaze::SharedEnttRegistry registry, sf::RenderWindow &window,
                Sprites::SpriteFactory &sprite_factory );

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

  void update_volume()
  {
    // get a copy of the component and assigns its value to the members
    auto effects_volume = get_persistent_component<Cmp::Persistent::EffectsVolume>();
    m_footsteps_sound_player.setVolume( effects_volume.get_value() );
  }
  void play_footsteps_sound();
  void stop_footsteps_sound();

private:
  sf::SoundBuffer m_footsteps_sound_buffer{ "res/audio/footsteps.wav" };
  sf::Sound m_footsteps_sound_player{ m_footsteps_sound_buffer };
};

} // namespace ProceduralMaze::Sys

#endif // __CMP_PLAYERSYSTEM_HPP__
