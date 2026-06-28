#ifndef SRC_COMPONENTS_PLAYER_PLAYERKEYSCOUNT_HPP__
#define SRC_COMPONENTS_PLAYER_PLAYERKEYSCOUNT_HPP__

namespace Game::Cmp
{

class PlayerKeysCount
{
public:
  PlayerKeysCount( unsigned int count )
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

#endif // SRC_COMPONENTS_PLAYER_PLAYERKEYSCOUNT_HPP__
