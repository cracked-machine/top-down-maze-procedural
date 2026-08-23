#ifndef SRC_COMPONENTS_CRYPT_SHUFFLETIMER_HPP__
#define SRC_COMPONENTS_CRYPT_SHUFFLETIMER_HPP__

namespace Game::Cmp::Crypt
{

//! @brief Singleton timer entity that drives how often Systems::CryptSystem re-shuffles the crypt maze's
//! room open/closed layout and passages while the maze remains locked.
class ShuffleTimer
{
public:
  //! @brief Construct a new shuffle timer with zero elapsed time.
  //! @param timeout Duration between maze shuffles.
  explicit ShuffleTimer( sf::Time timeout )
      : m_elapsed( sf::Time::Zero ),
        m_timeout( timeout )
  {
  }

  //! @brief Time accumulated since the last maze shuffle.
  sf::Time m_elapsed;
  //! @brief Duration between maze shuffles; a shuffle triggers once m_elapsed reaches this value.
  sf::Time m_timeout;
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_SHUFFLETIMER_HPP__
