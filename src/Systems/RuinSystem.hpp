#ifndef SRC_SYSTEMS_RUINSYSTEM_HPP_
#define SRC_SYSTEMS_RUINSYSTEM_HPP_

#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sys
{

class RuinSystem : public BaseSystem
{
public:
  RuinSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
              entt::dispatcher &scenemanager_event_dispatcher );

  //! @brief Update the player system.
  void update();

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

private:
  //! @brief Dispatcher reference for scene management events
  entt::dispatcher &m_scenemanager_event_dispatcher;
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_RUINSYSTEM_HPP_