#ifndef SRC_SYSTEMS_LOOTSYSTEM_HPP__
#define SRC_SYSTEMS_LOOTSYSTEM_HPP__

#include <Components/LootContainer.hpp>
#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/ReservedPosition.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sys
{

class LootSystem : public BaseSystem
{
public:
  LootSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  // Check for player collision with loot items i.e. player picks up loot
  void check_loot_collision();

private:
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_LOOTSYSTEM_HPP__