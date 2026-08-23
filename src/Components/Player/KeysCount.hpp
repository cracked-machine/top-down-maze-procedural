#ifndef SRC_COMPONENTS_PLAYER_KEYSCOUNT_HPP__
#define SRC_COMPONENTS_PLAYER_KEYSCOUNT_HPP__

namespace Game::Cmp::Player
{

//! @brief Tracks the number of keys the player currently holds.
class KeysCount
{
public:
  //! @brief Construct with an initial key count.
  //! @param count initial key count
  KeysCount( unsigned int count )
      : m_count( count )
  {
  }
  //! @brief Increase the key count.
  //! @param count amount to add to the current count
  void increment_count( unsigned int count ) { m_count += count; }
  //! @brief Decrease the key count, clamped at zero.
  //! @param count amount to subtract from the current count
  void decrement_count( unsigned int count )
  {
    if ( m_count == 0 ) return;
    m_count -= count;
  }

  //! @brief Get the current key count.
  //! @return unsigned int the current key count
  unsigned int get_count() const { return m_count; }

private:
  //! @brief Current key count.
  unsigned int m_count;
};

} // namespace Game::Cmp::Player

#endif // SRC_COMPONENTS_PLAYER_KEYSCOUNT_HPP__
