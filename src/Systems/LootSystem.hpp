#ifndef __SYS_LOOTSYSTEM_HPP__
#define __SYS_LOOTSYSTEM_HPP__

#include <Components/LootContainer.hpp>
#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/ReservedPosition.hpp>
#include <Events/LootContainerDestroyedEvent.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sys
{

class LootSystem : public BaseSystem
{
public:
  LootSystem( sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  // Check for player collision with loot items i.e. player picks up loot
  void check_loot_collision();

  void detonate_loot_container( const Events::LootContainerDestroyedEvent &event );

  /// EVENT SINKS ///
  void on_loot_container_destroyed( const Events::LootContainerDestroyedEvent &event )
  {
    SPDLOG_INFO( "Processing LootContainerDestroyedEvent for entity {}", static_cast<int>( event.m_entity ) );
    detonate_loot_container( event );
  }

private:
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_LOOTSYSTEM_HPP__