#ifndef __SYS_LOOTSYSTEM_HPP__
#define __SYS_LOOTSYSTEM_HPP__

#include <Components/LootContainer.hpp>
#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/ReservedPosition.hpp>
#include <Events/LootContainerDestroyedEvent.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sys {

class LootSystem : public BaseSystem
{
public:
  LootSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window,
              Sprites::SpriteFactory &sprite_factory );

  void detonate_loot_container( const Events::LootContainerDestroyedEvent &event );

  /// EVENT SINKS ///
  void on_loot_container_destroyed( const Events::LootContainerDestroyedEvent &event )
  {
    SPDLOG_INFO( "Processing LootContainerDestroyedEvent for entity {}", static_cast<int>( event.m_entity ) );
    detonate_loot_container( event );
  }

  void update_volume()
  {
    // get a copy of the component and assigns its value to the members
    auto effects_volume = get_persistent_component<Cmp::Persistent::EffectsVolume>();
    m_break_pot_sound_player.setVolume( effects_volume.get_value() );
  }

private:
  sf::SoundBuffer m_break_pot_sound_buffer{ "res/audio/break_pot.wav" };
  sf::Sound m_break_pot_sound_player{ m_break_pot_sound_buffer };
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_LOOTSYSTEM_HPP__