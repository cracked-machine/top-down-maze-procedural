#ifndef _SYS_DIGGING_SYSTEM_HPP__
#define _SYS_DIGGING_SYSTEM_HPP__

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
  DiggingSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory );

  void load_sounds();
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

  struct PickAxeSound
  {
    std::filesystem::path path;
    sf::SoundBuffer buffer;
  };
  std::vector<PickAxeSound> m_pickaxe_sounds{
      { "./res/audio/pickaxe1.mp3", sf::SoundBuffer() }, { "./res/audio/pickaxe2.mp3", sf::SoundBuffer() },
      { "./res/audio/pickaxe3.mp3", sf::SoundBuffer() }, { "./res/audio/pickaxe4.mp3", sf::SoundBuffer() },
      { "./res/audio/pickaxe5.mp3", sf::SoundBuffer() }, { "./res/audio/pickaxe6.mp3", sf::SoundBuffer() },

  };
  PickAxeSound m_pickaxe_final_sound{ "./res/audio/pickaxe_final.mp3", sf::SoundBuffer() };
  sf::Sound m_dig_sound{ m_pickaxe_sounds[0].buffer };
};
} // namespace ProceduralMaze::Sys

#endif // __SYSTEMS_DIGGING_SYSTEM_HPP__