#ifndef SRC_SYSTEMS_BOMBSYSTEM_HPP__
#define SRC_SYSTEMS_BOMBSYSTEM_HPP__

#include <Components/Persistent/EffectsVolume.hpp>
#include <entt/entity/entity.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <spdlog/spdlog.h>

#include <Components/Armed.hpp>
#include <Components/Loot.hpp>
#include <Components/NPC.hpp>
#include <Components/Neighbours.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/ArmedOffDelay.hpp>
#include <Components/Persistent/ArmedOnDelay.hpp>
#include <Components/Persistent/BombDamage.hpp>
#include <Components/Persistent/FuseDelay.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Position.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/Threats/NpcSystem.hpp>

namespace ProceduralMaze::Sys
{

// this currently only supports one bomb at a time
class BombSystem : public BaseSystem
{
public:
  BombSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief event handlers for pausing system clocks
  void onPause() override;
  //! @brief event handlers for resuming system clocks
  void onResume() override;

  void arm_occupied_location( const Events::PlayerActionEvent &event );
  void place_concentric_bomb_pattern( entt::entity &epicenter_entity, const int blast_radius );
  void update();

  /// EVENTS
  void on_player_action( const Events::PlayerActionEvent &event )
  {
    SPDLOG_DEBUG( "Player Action Event received" );
    if ( event.action == Events::PlayerActionEvent::GameActions::DROP_BOMB ) { arm_occupied_location( event ); }
    if ( event.action == Events::PlayerActionEvent::GameActions::GRAVE_BOMB ) { arm_occupied_location( event ); }
  }

private:
  const sf::Vector2f max_explosion_zone_size{ BaseSystem::kGridSquareSizePixels.x * 3.f, BaseSystem::kGridSquareSizePixels.y * 3.f };
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_BOMBSYSTEM_HPP__