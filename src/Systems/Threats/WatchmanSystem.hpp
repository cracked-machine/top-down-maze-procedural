#ifndef SRC_SYSTEMS_THREATS_WATCHMANSYSTEM_HPP__
#define SRC_SYSTEMS_THREATS_WATCHMANSYSTEM_HPP__

#include <Systems/BaseSystem.hpp>

#include <SFML/System/Time.hpp>

namespace Game::Sys
{

class WatchmanSystem : public BaseSystem
{
public:
  //! @brief Construct a new Watchman System object
  //! @param reg
  //! @param window
  //! @param sprite_factory
  //! @param sound_bank
  WatchmanSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief Rate-limit and trigger Watchman NPC spawning in the graveyard scene
  //! @param dt Delta time since last update call
  void update( sf::Time dt );

  //! @brief Create a watchman NPC at a random location.
  void spawn_watchman();

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

private:
  //! @brief Rate-limit the spawning of Watchman NPCs
  sf::Time m_watchman_spawn_timer;
};

} // namespace Game::Sys

#endif // SRC_SYSTEMS_THREATS_WATCHMANSYSTEM_HPP__
