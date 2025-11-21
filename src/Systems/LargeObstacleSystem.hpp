#ifndef __SYS_LARGEOBSTACLESYSTEM_HPP__
#define __SYS_LARGEOBSTACLESYSTEM_HPP__

#include <Components/LargeObstacle.hpp>
#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerCandlesCount.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/System/Clock.hpp>
#include <Systems/BaseSystem.hpp>
#include <entt/entity/fwd.hpp>

namespace ProceduralMaze::Sys
{

class LargeObstacleSystem : public BaseSystem
{
public:
  LargeObstacleSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                       Audio::SoundBank &sound_bank );

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  void check_player_lo_collision( Events::PlayerActionEvent::GameActions action );
  void check_player_shrine_activation( Cmp::LargeObstacle &lo_cmp, Cmp::PlayerCandlesCount &pc_candles_cmp );
  void check_player_grave_activation( Cmp::LargeObstacle &lo_cmp, entt::entity &lo_entity, Cmp::PlayableCharacter &pc_cmp );
  bool activate_shrine_special_power();

  /// EVENT SINKS ///
  void on_player_action( const Events::PlayerActionEvent &event )
  {
    if ( event.action == Events::PlayerActionEvent::GameActions::ACTIVATE ) { check_player_lo_collision( event.action ); }
  }

private:
  sf::Clock m_shrine_activation_clock;
  sf::Time m_shrine_activation_cooldown{ sf::seconds( 3.f ) };
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_LARGEOBSTACLESYSTEM_HPP__