#ifndef SRC_SYSTEMS_SHOPSYSTEM_HPP_
#define SRC_SYSTEMS_SHOPSYSTEM_HPP_

#include <Components/HolyWell/HolyWellMultiBlock.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sys
{

class ShopSystem : public BaseSystem
{
public:
  ShopSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
              entt::dispatcher &scenemanager_event_dispatcher )
      : ProceduralMaze::Sys::BaseSystem( reg, window, sprite_factory, sound_bank ),
        m_scenemanager_event_dispatcher( scenemanager_event_dispatcher )
  {
  }

  void spawn_exit( sf::Vector2u spawn_position );

  //! @brief Check for collisions with the exit
  void check_exit_collision();

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

private:
  //! @brief Dispatcher reference for scene management events
  entt::dispatcher &m_scenemanager_event_dispatcher;
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_SHOPSYSTEM_HPP_