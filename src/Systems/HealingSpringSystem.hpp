#ifndef SRC_SYSTEMS_SACREDSPRINGSYSTEM_HPP__
#define SRC_SYSTEMS_SACREDSPRINGSYSTEM_HPP__

#include <Events/PlayerActionEvent.hpp>
#include <Systems/BaseSystem.hpp>

namespace Game::Sys
{

class HealingSpringSystem : public BaseSystem
{
public:
  HealingSpringSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
                       entt::dispatcher &scenemanager_event_dispatcher )
      : Game::Sys::BaseSystem( reg, window, sprite_factory, sound_bank ),
        m_scenemanager_event_dispatcher( scenemanager_event_dispatcher )
  {
    std::ignore = get_systems_event_queue().sink<Events::PlayerActionEvent>().connect<&HealingSpringSystem::on_player_action>( this );
  }

  void on_player_action( Events::PlayerActionEvent ev );

  void update_building_zorder();

  //! @brief Check for collisions with the entrance
  void check_entrance_collision();
  void check_exit_collision();
  void check_inventory_deposit();

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

private:
  //! @brief Dispatcher reference for scene management events
  entt::dispatcher &m_scenemanager_event_dispatcher;

  sf::Time m_inventory_deposit_interval{};
};

} // namespace Game::Sys

#endif // SRC_SYSTEMS_SACREDSPRINGSYSTEM_HPP__
