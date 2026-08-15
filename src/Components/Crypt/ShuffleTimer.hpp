#ifndef SRC_COMPONENTS_CRYPT_SHUFFLETIMER_HPP__
#define SRC_COMPONENTS_CRYPT_SHUFFLETIMER_HPP__

namespace Game::Cmp::Crypt
{

class ShuffleTimer
{
public:
  explicit ShuffleTimer( sf::Time timeout )
      : m_elapsed( sf::Time::Zero ),
        m_timeout( timeout )
  {
  }

  sf::Time m_elapsed;
  sf::Time m_timeout;
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_SHUFFLETIMER_HPP__
