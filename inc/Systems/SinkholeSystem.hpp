#ifndef __SYS_SINKHOLESYSTEM_HPP__
#define __SYS_SINKHOLESYSTEM_HPP__

#include <BaseSystem.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
namespace ProceduralMaze::Sys {

class SinkholeSystem : public Sys::BaseSystem
{
public:
  SinkholeSystem( ProceduralMaze::SharedEnttRegistry reg )
      : Sys::BaseSystem( reg )
  {
  }

  // Starts the sinkhole process, gets view of all obstacles, adds one sinkhole component at one of the positions.
  // This is done only once at the start of the game so it will check if there is already a sinkhole component
  // present.
  void start_sinkhole();

  // add an adjacent sinkhole component if possible
  // This is done every m_interval seconds, using the m_clock to track time.
  // get a view of all sinkholes, for each sinkhole check adjacent positions
  // if there is no sinkhole, add a sinkhole component
  void update_sinkhole();

private:
  sf::Clock m_clock;
  const sf::Time m_interval_sec{ sf::seconds( 5.0f ) }; // seconds between adding new sinkholes
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_SINKHOLESYSTEM_HPP__