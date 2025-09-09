#ifndef __COMPONENTS_BOMB_SYSTEM_HPP__
#define __COMPONENTS_BOMB_SYSTEM_HPP__

#include <Components/Armed.hpp>
#include <Components/Loot.hpp>
#include <Components/Movement.hpp>
#include <Components/NPC.hpp>
#include <Components/Neighbours.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Position.hpp>
#include <Events/NpcDeathEvent.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <NPCScanBounds.hpp>
#include <NpcSystem.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/BaseSystem.hpp>

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <entt/entity/entity.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>

#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys {

// this currently only supports one bomb at a time
class BombSystem : public BaseSystem
{
public:
  BombSystem( std::shared_ptr<entt::basic_registry<entt::entity>> reg ) : BaseSystem( reg ) {}

  void suspend();
  void resume();

  void arm_occupied_location();
  void place_concentric_bomb_pattern( entt::entity &epicenter_entity, const int BLAST_RADIUS );
  void update();

  /// EVENTS
  void on_player_action( const Events::PlayerActionEvent &event )
  {
    SPDLOG_DEBUG( "Player Action Event received" );
    if ( event.action == Events::PlayerActionEvent::GameActions::DROP_BOMB )
    {
      arm_occupied_location();
    }
  }

private:
  struct Settings
  {
    float base_fuse_delay = 3.0f;
    float armed_detonation_delay_increment = 0.025f;
    float armed_warning_delay_increment = 0.75f;
    int player_damage = 10; // Amount of damage to deal to the player when hit by explosion
  };

  const sf::Vector2f max_explosion_zone_size{
      Sprites::SpriteFactory::DEFAULT_SPRITE_SIZE.x * 3.f,
      Sprites::SpriteFactory::DEFAULT_SPRITE_SIZE.y * 3.f
  };

  sf::SoundBuffer m_fuse_sound_buffer{ "res/audio/fuse.wav" };
  sf::Sound m_fuse_sound_player{ m_fuse_sound_buffer };
  sf::SoundBuffer m_detonate_sound_buffer{ "res/audio/detonate.wav" };
  sf::Sound m_detonate_sound_player{ m_detonate_sound_buffer };

public:
  BombSystem::Settings m_settings;
};

} // namespace ProceduralMaze::Sys

#endif // __COMPONENTS_BOMB_SYSTEM_HPP__