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
  ExitSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window,
              Sprites::SpriteFactory &sprite_factory );

  void spawn_exit();
  void unlock_exit();

  // Check for player collision with exit to complete level
  void check_exit_collision();

  /// EVENT SINKS ///
  void on_door_unlock_event( [[maybe_unused]] const Events::UnlockDoorEvent &event ) { unlock_exit(); }

  void update_volume()
  {
    // get a copy of the component and assigns its value to the members
    auto effects_volume = get_persistent_component<Cmp::Persistent::EffectsVolume>();
    exit_unlock_sound_player.setVolume( effects_volume.get_value() );
  }

private:
  int exit_sprite_index = 6;
  sf::SoundBuffer exit_unlock_sound_buffer{ "res/audio/secret.wav" };
  sf::Sound exit_unlock_sound_player{ exit_unlock_sound_buffer };
};

} // namespace ProceduralMaze::Sys

#endif // _SYS_EXIT_SYSTEM_HPP__