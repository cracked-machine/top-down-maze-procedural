#ifndef SRC_SYSTEMS_ALTARSYSTEM_HPP__
#define SRC_SYSTEMS_ALTARSYSTEM_HPP__

#include <Components/AltarMultiBlock.hpp>
#include <Components/PlayerCandlesCount.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sys
{

class AltarSystem : public BaseSystem
{
public:
  AltarSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  void on_player_action( const Events::PlayerActionEvent &event )
  {
    if ( event.action == Events::PlayerActionEvent::GameActions::ACTIVATE ) { check_player_collision( event.action ); }
  }

private:
  void check_player_collision( Events::PlayerActionEvent::GameActions action );
  void check_player_altar_activation( entt::entity altar_entity, Cmp::AltarMultiBlock &altar_cmp, Cmp::PlayerCandlesCount &pc_candles_cmp );
  bool activate_altar_special_power();

  sf::Clock m_altar_activation_clock;
  sf::Time m_altar_activation_cooldown{ sf::seconds( 3.f ) };
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_ALTARSYSTEM_HPP__