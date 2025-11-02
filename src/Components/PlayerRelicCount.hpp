#ifndef __COMPONENTS_PLAYER_RELIC_COUNT_HPP__
#define __COMPONENTS_PLAYER_RELIC_COUNT_HPP__

namespace ProceduralMaze::Cmp {

class PlayerRelicCount
{
public:
  PlayerRelicCount( unsigned int count )
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

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_PLAYER_RELIC_COUNT_HPP__