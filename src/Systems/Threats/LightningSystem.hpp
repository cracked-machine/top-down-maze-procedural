#ifndef SRC_SYSTEM_THREATS_LIGHTNINGSYSTEM_HPP_
#define SRC_SYSTEM_THREATS_LIGHTNINGSYSTEM_HPP_

#include <Events/LightningEvent.hpp>
#include <Components/LightningStrike.hpp>
#include <Systems/BaseSystem.hpp>

namespace Game::Cmp
{
class LightningStrike;
}

namespace Game::Sys
{

class LightningSystem : public BaseSystem
{
public:
  //! @brief Construct a new Lightning System object
  //! @param reg
  //! @param window
  //! @param sprite_factory
  //! @param sound_bank
  LightningSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief Sets `trigger_lightning` flag to true;
  //! @param ev
  void on_lightning_event( Events::LightningEvent ev );

  //! @brief Called every frame by the Graveyard Scene
  //! @param dt
  void update( sf::Time dt );

  //! @brief Checks if their is existing Cmp::LightningStrike entity in the entt registry
  //! @return true
  //! @return false
  bool lightning_strike_exists();

  //! @brief event handlers for pausing system clocks
  void on_pause() override {};
  //! @brief event handlers for resuming system clocks
  void on_resume() override {};

private:
  //! @brief
  bool trigger_lightning{ false };

  //! @brief Create a lightning strike object
  //! @param dt
  void create_lightning_strike( sf::Time dt );

  //! @brief Used to create the lightning strike "zig-zag" sections. Called N times by LightningSystem::create_lightning_strike().
  //! @note It takes each existing segment and divides into two segments. The dividing vertex point is then offset at perpendicular distance
  //! @param ls_seq
  //! @param deviations
  //! @param lines_per_strike
  void divide_lightning_segments( std::vector<std::vector<sf::Vertex>> &ls_seq, Cmp::LightningStrike::AngleDeviations deviations,
                                  int lines_per_strike );

  //! @brief If the Cmp::LightningStrike timer has expired then delete the entity
  void delete_expired_lightning_strikes();
};

} // namespace Game::Sys

#endif // SRC_SYSTEM_THREATS_LIGHTNINGSYSTEM_HPP_