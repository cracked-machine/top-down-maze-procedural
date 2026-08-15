#ifndef SRC_COMPONENTS_PLAYER_LEVELDEPTH_HPP__
#define SRC_COMPONENTS_PLAYER_LEVELDEPTH_HPP__

#include <SFML/System/Time.hpp>

namespace Game::Cmp::Player
{

class LevelDepth
{
public:
  LevelDepth( unsigned int level )
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

} // namespace Game::Cmp::Player

#endif // SRC_COMPONENTS_PLAYER_LEVELDEPTH_HPP__
