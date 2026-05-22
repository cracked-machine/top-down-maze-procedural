#ifndef __COMPONENTS_PLAYER_CADAVER_COUNT_HPP__
#define __COMPONENTS_PLAYER_CADAVER_COUNT_HPP__

namespace Game::Cmp
{

class PlayerCadaverCount
{
public:
  PlayerCadaverCount( unsigned int count )
      : m_count( count )
  {
  }
  void increment_count( unsigned int count ) { m_count += count; }
  void decrement_count( unsigned int count )
  {
    if ( m_count == 0 ) return;
    m_count -= count;
  }

  unsigned int get_count() const { return m_count; }

private:
  unsigned int m_count;
};

} // namespace Game::Cmp

#endif // __COMPONENTS_PLAYER_CADAVER_COUNT_HPP__