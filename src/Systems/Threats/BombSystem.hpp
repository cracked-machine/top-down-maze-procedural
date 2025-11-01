#ifndef __COMPONENTS_BOMB_SYSTEM_HPP__
#define __COMPONENTS_BOMB_SYSTEM_HPP__

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
#include <Components/NPCScanBounds.hpp>
#include <Components/Neighbours.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/ArmedOffDelay.hpp>
#include <Components/Persistent/ArmedOnDelay.hpp>
#include <Components/Persistent/BombDamage.hpp>
#include <Components/Persistent/FuseDelay.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Position.hpp>
#include <Events/NpcDeathEvent.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/Threats/NpcSystem.hpp>

namespace ProceduralMaze::Sys {

// this currently only supports one bomb at a time
class BombSystem : public BaseSystem
{
public:
  BombSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window,
              Sprites::SpriteFactory &sprite_factory );

  void suspend();
  void resume();

  void arm_occupied_location();
  void place_concentric_bomb_pattern( entt::entity &epicenter_entity, const int blast_radius );
  void update();

  /// EVENTS
  void on_player_action( const Events::PlayerActionEvent &event )
  {
    SPDLOG_DEBUG( "Player Action Event received" );
    if ( event.action == Events::PlayerActionEvent::GameActions::DROP_BOMB ) { arm_occupied_location(); }
  }

  void update_volume()
  {
    // get a copy of the component and assigns its value to the members
    auto effects_volume = get_persistent_component<Cmp::Persistent::EffectsVolume>();
    m_fuse_sound_player.setVolume( effects_volume.get_value() );
    m_detonate_sound_player.setVolume( effects_volume.get_value() );
  }

private:
  const sf::Vector2f max_explosion_zone_size{ BaseSystem::kGridSquareSizePixels.x * 3.f,
                                              BaseSystem::kGridSquareSizePixels.y * 3.f };

  sf::SoundBuffer m_fuse_sound_buffer{ "res/audio/fuse.wav" };
  sf::Sound m_fuse_sound_player{ m_fuse_sound_buffer };
  sf::SoundBuffer m_detonate_sound_buffer{ "res/audio/detonate.wav" };
  sf::Sound m_detonate_sound_player{ m_detonate_sound_buffer };
};

} // namespace ProceduralMaze::Sys

#endif // __COMPONENTS_BOMB_SYSTEM_HPP__