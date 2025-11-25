#ifndef SRC_SYSTEMS_CRYPTSYSTEM_HPP__
#define SRC_SYSTEMS_CRYPTSYSTEM_HPP__

#include <Components/CryptDoor.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sys
{

class CryptSystem : public ProceduralMaze::Sys::BaseSystem
{
public:
  CryptSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
      : ProceduralMaze::Sys::BaseSystem( reg, window, sprite_factory, sound_bank )
  {
    // The entt::dispatcher is independent of the registry, so it is safe to bind event handlers in the constructor
    std::ignore = get_systems_event_queue().sink<Events::PlayerActionEvent>().connect<&CryptSystem::on_player_action>( this );
  }

  void on_player_action( Events::PlayerActionEvent &event )
  {
    // Handle player actions related to crypts here if needed
    // For now, this function is a placeholder
    if ( event.action == Events::PlayerActionEvent::GameActions::ACTIVATE ) crypt_door_collisions();
  }
  virtual void onPause() override {}
  virtual void onResume() override {}
  void crypt_door_collisions();

private:
  sf::Clock m_door_cooldown_timer;
  float m_door_cooldown_time{ 1.0f }; // 1 second cooldown
};

} // namespace ProceduralMaze::Sys
#endif // SRC_SYSTEMS_CRYPTSYSTEM_HPP__