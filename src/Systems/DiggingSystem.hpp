#ifndef SRC_SYSTEMS_DIGGINGSYSTEM_HPP__
#define SRC_SYSTEMS_DIGGINGSYSTEM_HPP__

#include <Components/Persistent/EffectsVolume.hpp>
#include <SFML/Audio/AudioResource.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/System/Clock.hpp>

#include <Events/PlayerActionEvent.hpp>
#include <Systems/BaseSystem.hpp>

#include <filesystem>
namespace ProceduralMaze::Sys
{

// DiggingSystem handles player digging actions within the maze.
// This system is mainly event-driven, responding to player dig actions.
// However, there is also a periodic update to clear previous dig selections,
// if the cooldown has expired.
class DiggingSystem : public BaseSystem
{
public:
  DiggingSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  // void load_sounds();
  // additional updates via the main game loop
  void update();

  // Event handler for player actions
  void on_player_action( const Events::PlayerActionEvent &event );

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

  // Cooldown clock to manage digging intervals
  sf::Clock m_dig_cooldown_clock;

  //! @brief Structure to hold pickaxe sound information
  //! Used to manage multiple pickaxe sound effects
  struct PickAxeSound
  {
    std::filesystem::path path;
    sf::SoundBuffer buffer;
  };
};
} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_DIGGINGSYSTEM_HPP__