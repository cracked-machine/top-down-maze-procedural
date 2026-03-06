#ifndef SRC_SYSTEMS_PATHSYSTEM_HPP_
#define SRC_SYSTEMS_PATHSYSTEM_HPP_

#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Cmp
{
class Position;
}

namespace ProceduralMaze::Sys
{

class PathSystem : public BaseSystem
{
public:
  PathSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  void update( sf::Time dt );

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

private:
  sf::Time m_scan_accumulator;
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_PATHSYSTEM_HPP_