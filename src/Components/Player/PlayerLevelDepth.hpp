#ifndef SRC_CMPS_PLAYER_PLAYERLEVELDEPTH_HPP_
#define SRC_CMPS_PLAYER_PLAYERLEVELDEPTH_HPP_

#include <SFML/System/Time.hpp>

namespace ProceduralMaze::Cmp
{

class PlayerLevelDepth
{
public:
  PlayerLevelDepth( unsigned int level )
      : m_level( level )
  {
  }
  void increment_count( unsigned int count ) { m_level += count; }
  void decrement_count( unsigned int count )
  {
    if ( m_level == 0 ) return;
    m_level -= count;
  }

  unsigned int get_count() const { return m_level; }

  sf::Time display_cooldown{ sf::seconds( 5 ) };
  sf::Clock display_timer;

private:
  unsigned int m_level;
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_PLAYER_PLAYERLEVELDEPTH_HPP_