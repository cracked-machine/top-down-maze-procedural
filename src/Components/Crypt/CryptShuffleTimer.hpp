#ifndef SRC_CMPS_CRYPT_CRYPTSHUFFLETIMER_HPP_
#define SRC_CMPS_CRYPT_CRYPTSHUFFLETIMER_HPP_

namespace Game::Cmp
{

class CryptShuffleTimer : public sf::Clock
{
public:
  CryptShuffleTimer( sf::Time timeout )
      : m_timeout( timeout )
  {
  }

  sf::Time m_timeout;
};

} // namespace Game::Cmp

#endif // SRC_CMPS_CRYPT_CRYPTSHUFFLETIMER_HPP_