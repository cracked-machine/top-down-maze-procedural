#ifndef __SYS_FOOTSTEPSYSTEM_HPP__
#define __SYS_FOOTSTEPSYSTEM_HPP__

#include <Direction.hpp>
#include <Position.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sys {

class FootstepSystem : public BaseSystem
{
public:
  FootstepSystem( ProceduralMaze::SharedEnttRegistry reg )
      : BaseSystem( reg )
  {
  }
  // create an entity with components: Position, Direction, FootstepTimer, FootstepAlpha
  void add_footstep( const Cmp::Position &position, const Cmp::Direction &direction );
  // Update all FootstepAlpha based on their FootstepTimer, remove any entities with FootstepAlpha
  // <= 0
  void update();

  constexpr static const sf::Vector2f kFootstepSize{ 10.f, 6.f };

private:
  const sf::Time kFootstepFadeDelay{ sf::seconds( 1.f ) };
  const sf::Time kFootstepAddDelay{ sf::seconds( 0.2f ) };
  const unsigned int kFootstepFadeFactor{ 1 };
  sf::Clock update_clock{};
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_FOOTSTEPSYSTEM_HPP__
