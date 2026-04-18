#ifndef SRC_SYSTEM_THREATS_LIGHTNINGSYSTEM_HPP_
#define SRC_SYSTEM_THREATS_LIGHTNINGSYSTEM_HPP_

#include <Events/LightningEvent.hpp>
#include <LightningStrike.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Cmp
{
class LightningStrike;
}

namespace ProceduralMaze::Sys
{

class LightningSystem : public BaseSystem
{
public:
  LightningSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );
  void on_lightning_event( [[maybe_unused]] Events::LightningEvent ev );

  void update( sf::Time dt );

  bool lightning_strike_exists();
  void create_lightning_strike( sf::Time dt );
  void delete_expired_lightning_strikes();
  void divide_lightning_segments( std::vector<std::vector<sf::Vertex>> &ls_seq, Cmp::LightningStrike::AngleDeviations deviations,
                                  int lines_per_strike );

  //! @brief event handlers for pausing system clocks
  void on_pause() override {};
  //! @brief event handlers for resuming system clocks
  void on_resume() override {};

private:
  bool trigger_lightning{ false };
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEM_THREATS_LIGHTNINGSYSTEM_HPP_