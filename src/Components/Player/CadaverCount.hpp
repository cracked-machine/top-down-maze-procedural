#ifndef SRC_COMPONENTS_PLAYER_CADAVERCOUNT_HPP__
#define SRC_COMPONENTS_PLAYER_CADAVERCOUNT_HPP__

namespace Game::Cmp::Player
{

//! @brief Tracks the number of cadavers the player has collected/interacted with at graves.
class CadaverCount
{
public:
  //! @brief Construct with an initial cadaver count.
  //! @param count initial cadaver count
  CadaverCount( unsigned int count )
      : m_count( count )
  {
  }
  //! @brief Increase the cadaver count.
  //! @param count amount to add to the current count
  void increment_count( unsigned int count ) { m_count += count; }
  //! @brief Decrease the cadaver count, clamped at zero.
  //! @param count amount to subtract from the current count
  void decrement_count( unsigned int count )
  {
    if ( m_count == 0 ) return;
    m_count -= count;
  }

  //! @brief Get the current cadaver count.
  //! @return unsigned int the current cadaver count
  unsigned int get_count() const { return m_count; }

private:
  //! @brief Current cadaver count.
  unsigned int m_count;
};

} // namespace Game::Cmp::Player

#endif // SRC_COMPONENTS_PLAYER_CADAVERCOUNT_HPP__
