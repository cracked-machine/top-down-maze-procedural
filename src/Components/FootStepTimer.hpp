#ifndef __CMP_FOOTSTEPTIMER_HPP__
#define __CMP_FOOTSTEPTIMER_HPP__

#include <SFML/System/Clock.hpp>

namespace Game::Cmp
{

struct FootStepTimer
{
  sf::Clock m_clock;
};

} // namespace Game::Cmp

#endif // __CMP_FOOTSTEPTIMER_HPP__