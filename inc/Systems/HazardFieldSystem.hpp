#ifndef __SYS_HAZARDFIELDSYSTEM_HPP__
#define __SYS_HAZARDFIELDSYSTEM_HPP__

#include <BaseSystem.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
namespace ProceduralMaze::Sys {

class HazardFieldSystem : public Sys::BaseSystem
{
public:
  HazardFieldSystem( ProceduralMaze::SharedEnttRegistry reg )
      : Sys::BaseSystem( reg )
  {
  }

  // Starts the hazard field process, gets view of all obstacles, adds one hazard field component at one of the
  // positions. This is done only once at the start of the game so it will check if there is already a hazard field
  // component present.
  void start_hazard_field();

  // add an adjacent hazard field component if possible
  // This is done every m_interval seconds, using the m_clock to track time.
  // get a view of all hazard fields, for each hazard field check adjacent positions
  // if there is no hazard field, add a hazard field component
  void update_hazard_field();

private:
  sf::Clock m_clock;
  const sf::Time m_interval_sec{ sf::seconds( 5.0f ) }; // seconds between adding new hazard fields
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_HAZARDFIELDSYSTEM_HPP__