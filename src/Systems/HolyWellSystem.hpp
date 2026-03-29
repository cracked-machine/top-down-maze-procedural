#ifndef SRC_SYSTEMS_HOLYWELLSYSTEM_HPP__
#define SRC_SYSTEMS_HOLYWELLSYSTEM_HPP__

#include <Components/HolyWell/HolyWellMultiBlock.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sys
{

class HolyWellSystem : public BaseSystem
{
public:
  HolyWellSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
                  entt::dispatcher &scenemanager_event_dispatcher )
      : ProceduralMaze::Sys::BaseSystem( reg, window, sprite_factory, sound_bank ),
        m_scenemanager_event_dispatcher( scenemanager_event_dispatcher )
  {
    std::ignore = get_systems_event_queue().sink<Events::PlayerActionEvent>().connect<&HolyWellSystem::on_player_action>( this );
  }

  void on_player_action( Events::PlayerActionEvent ev );

  void spawn_well( sf::Vector2u spawn_position );

  //! @brief Check for collisions with the entrance
  void check_entrance_collision();
  void check_exit_collision();
  void check_inventory_deposit();

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

private:
  //! @brief Dispatcher reference for scene management events
  entt::dispatcher &m_scenemanager_event_dispatcher;

  sf::Time m_inventory_deposit_interval{};
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_HOLYWELLSYSTEM_HPP__