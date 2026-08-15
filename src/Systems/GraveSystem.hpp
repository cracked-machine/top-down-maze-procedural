#ifndef SRC_SYSTEMS_GRAVESYSTEM_HPP__
#define SRC_SYSTEMS_GRAVESYSTEM_HPP__

#include <Components/Grave/MultiBlock.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Systems/BaseSystem.hpp>

namespace Game::Cmp::Player
{
class Character;
}

namespace Game::Sys
{

class GraveSystem : public BaseSystem
{
public:
  GraveSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

  void on_player_action( const Events::PlayerActionEvent &event );
  void check_player_grave_collision();

  // Cooldown clock to manage digging intervals
  sf::Clock m_dig_cooldown_clock;
};

} // namespace Game::Sys

#endif // SRC_SYSTEMS_GRAVESYSTEM_HPP__
