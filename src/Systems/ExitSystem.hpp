#ifndef SRC_SYSTEMS_EXITSYSTEM_HPP__
#define SRC_SYSTEMS_EXITSYSTEM_HPP__

#include <Events/PlayerActionEvent.hpp>
#include <Events/UnlockDoorEvent.hpp>
#include <Systems/BaseSystem.hpp>

#include <SFML/System/Vector2.hpp>

namespace Game::Sys
{

class ExitSystem : public BaseSystem
{
public:
  ExitSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
              entt::dispatcher &scenemanager_event_dispatcher );

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

  //! @brief Event handler for unlocking the exit
  //! @param ev
  void on_player_action( Events::PlayerActionEvent ev );

  //! @brief Create the exit in the game area
  //! @param spawn_position
  void spawn_exit();

  //! @brief  This function is called via event trigger:
  //!         `Events::PlayerActionEvent` from `SceneInputRouter::graveyard_scene_state_handler()`
  void unlock_exit();

  //! @brief Check for player collision with open exit to complete level
  void check_exit_collision();

  //! @brief Update exit multiblock z-order so it renders in front of the player unless the player is nearby
  void update_exit_zorder();

private:
  int exit_sprite_index = 6;
  entt::dispatcher &m_scenemanager_event_dispatcher;
};

} // namespace Game::Sys

#endif // SRC_SYSTEMS_EXITSYSTEM_HPP__
