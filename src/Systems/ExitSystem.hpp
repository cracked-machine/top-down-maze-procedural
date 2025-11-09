#ifndef _SYS_EXIT_SYSTEM_HPP__
#define _SYS_EXIT_SYSTEM_HPP__

#include <Components/Persistent/EffectsVolume.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <Events/UnlockDoorEvent.hpp>
#include <Systems/BaseSystem.hpp>
namespace ProceduralMaze::Sys {

class ExitSystem : public BaseSystem
{
public:
  ExitSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
              Audio::SoundBank &sound_bank );

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  void spawn_exit();
  void unlock_exit();

  // Check for player collision with exit to complete level
  void check_exit_collision();

  /// EVENT SINKS ///
  void on_door_unlock_event( [[maybe_unused]] const Events::UnlockDoorEvent &event ) { unlock_exit(); }

private:
  int exit_sprite_index = 6;
};

} // namespace ProceduralMaze::Sys

#endif // _SYS_EXIT_SYSTEM_HPP__