#ifndef SRC_SYSTEMS_FOOTSTEPSYSTEM_HPP__
#define SRC_SYSTEMS_FOOTSTEPSYSTEM_HPP__

#include <Components/FootStepTimer.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>

#include <Components/Direction.hpp>
#include <Components/Position.hpp>
#include <Systems/BaseSystem.hpp>

namespace Game::Sys
{

class FootstepSystem : public BaseSystem
{
public:
  FootstepSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
      : BaseSystem( reg, window, sprite_factory, sound_bank )
  {
  }

  enum class FootStepSfx { NONE, GRAVEL, FLOORBOARDS };

  // Update all FootstepAlpha based on their FootstepTimer, remove any entities with FootstepAlpha
  void update( FootStepSfx footstep_sfx = FootStepSfx::GRAVEL );

  //! @brief event handlers for pausing footstep clocks
  void on_pause() override;
  //! @brief event handlers for resuming footstep clocks
  void on_resume() override;

  //! @brief
  void stop_footsteps_sound();

private:
  //! @brief Rate-limit adding the footstep sprites
  sf::Clock update_clock;

  // create an entity with components: Position, Direction, FootstepTimer, FootstepAlpha
  void add_footstep( const Cmp::Position &pos, const Cmp::Direction &dir );

  //! @brief
  //! @param type
  void play_footsteps_sound( FootStepSfx type );
};

} // namespace Game::Sys

#endif // SRC_SYSTEMS_FOOTSTEPSYSTEM_HPP__
