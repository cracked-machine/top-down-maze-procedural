#ifndef SRC_SYSTEMS_EXITSYSTEM_HPP__
#define SRC_SYSTEMS_EXITSYSTEM_HPP__

#include <Components/Persistent/EffectsVolume.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <Events/UnlockDoorEvent.hpp>
#include <SFML/System/Vector2.hpp>
#include <Systems/BaseSystem.hpp>
namespace ProceduralMaze::Sys
{

class ExitSystem : public BaseSystem
{
public:
  ExitSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
              entt::dispatcher &scenemanager_event_dispatcher );

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  //! @brief Event handler for unlocking the exit
  //! @param ev
  void on_player_action( Events::PlayerActionEvent ev );

  //! @brief Create the exit in the game area
  //! @param spawn_position
  void spawn_exit( std::optional<sf::Vector2u> spawn_position = std::nullopt );

  //! @brief  This function is called via event trigger:
  //!         `Events::PlayerActionEvent` from `SceneInputRouter::graveyard_scene_state_handler()`
  void check_player_can_unlock_exit();

  //! @brief Check for player collision with open exit to complete level
  void check_exit_collision();

private:
  int exit_sprite_index = 6;
  entt::dispatcher &m_scenemanager_event_dispatcher;
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_EXITSYSTEM_HPP__