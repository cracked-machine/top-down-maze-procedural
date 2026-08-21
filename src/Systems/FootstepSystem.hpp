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

//! @brief Spawns and fades footstep sprites behind the player, and plays/stops the matching footstep sound.
class FootstepSystem : public BaseSystem
{
public:
  //! @brief Construct a new Footstep System object
  //! @param reg
  //! @param window
  //! @param sprite_factory
  //! @param sound_bank
  FootstepSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
      : BaseSystem( reg, window, sprite_factory, sound_bank )
  {
  }

  enum class FootStepSfx { NONE, GRAVEL, FLOORBOARDS };

  //! @brief Update all FootstepAlpha based on their FootstepTimer, remove any entities with FootstepAlpha
  //! @param footstep_sfx
  void update( FootStepSfx footstep_sfx = FootStepSfx::GRAVEL );

  //! @brief event handlers for pausing footstep clocks
  void on_pause() override;
  //! @brief event handlers for resuming footstep clocks
  void on_resume() override;

  //! @brief Stop the currently playing footstep sound.
  void stop_footsteps_sound();

private:
  //! @brief Rate-limit adding the footstep sprites
  sf::Clock update_clock;

  //! @brief Create an entity with components: Position, Direction, FootstepTimer, FootstepAlpha
  void add_footstep( const Cmp::Position &pos, const Cmp::Direction &dir );

  //! @brief Play the looping footstep sound matching `type`, if not already playing.
  //! @param type
  void play_footsteps_sound( FootStepSfx type );
};

} // namespace Game::Sys

#endif // SRC_SYSTEMS_FOOTSTEPSYSTEM_HPP__
