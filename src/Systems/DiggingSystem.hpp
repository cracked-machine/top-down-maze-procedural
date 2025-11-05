#ifndef _SYS_DIGGING_SYSTEM_HPP__
#define _SYS_DIGGING_SYSTEM_HPP__

#include <Components/Persistent/EffectsVolume.hpp>
#include <SFML/Audio/AudioResource.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/System/Clock.hpp>

#include <Events/PlayerActionEvent.hpp>
#include <Systems/BaseSystem.hpp>

#include <filesystem>
namespace ProceduralMaze::Sys {

// DiggingSystem handles player digging actions within the maze.
// This system is mainly event-driven, responding to player dig actions.
// However, there is also a periodic update to clear previous dig selections,
// if the cooldown has expired.
class DiggingSystem : public BaseSystem
{
public:
  DiggingSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window,
                 Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

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

  // Cooldown clock to manage digging intervals
  sf::Clock m_dig_cooldown_clock;

  //! @brief Structure to hold pickaxe sound information
  //! Used to manage multiple pickaxe sound effects
  struct PickAxeSound
  {
    std::filesystem::path path;
    sf::SoundBuffer buffer;
  };

  // //! @brief List of non-final pickaxe sound effects
  // //! These dont include the final shatter sound. These are randomly selected to create audible variety, if the
  // obstacle
  // //! is not yet destroyed.
  // std::vector<PickAxeSound> m_pickaxe_sounds{
  //     { "./res/audio/pickaxe1.wav", sf::SoundBuffer() }, { "./res/audio/pickaxe2.wav", sf::SoundBuffer() },
  //     { "./res/audio/pickaxe3.wav", sf::SoundBuffer() }, { "./res/audio/pickaxe4.wav", sf::SoundBuffer() },
  //     { "./res/audio/pickaxe5.wav", sf::SoundBuffer() }, { "./res/audio/pickaxe6.wav", sf::SoundBuffer() },

  // };

  // //! @brief Final pickaxe sound effect
  // //! This sound is played when the last hit destroys an obstacle. This is separate from the other sounds for
  // //! convenience only.
  // PickAxeSound m_pickaxe_final_sound{ "./res/audio/pickaxe_final.wav", sf::SoundBuffer() };

  // //! @brief Digging sound player
  // //! Controls the randomly selected PickAxeSound playback. Defaults to the first pickaxe sound.
  // sf::Sound m_dig_sound_player{ m_pickaxe_sounds[0].buffer };
};
} // namespace ProceduralMaze::Sys

#endif // __SYSTEMS_DIGGING_SYSTEM_HPP__