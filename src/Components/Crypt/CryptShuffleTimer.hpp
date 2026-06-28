#ifndef SRC_COMPONENTS_CRYPT_CRYPTSHUFFLETIMER_HPP__
#define SRC_COMPONENTS_CRYPT_CRYPTSHUFFLETIMER_HPP__

namespace Game::Cmp
{

class CryptShuffleTimer
{
public:
  explicit CryptShuffleTimer( sf::Time timeout )
      : m_elapsed( sf::Time::Zero ),
        m_timeout( timeout )
  {
  }

  sf::Time m_elapsed;
  sf::Time m_timeout;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_CRYPT_CRYPTSHUFFLETIMER_HPP__
