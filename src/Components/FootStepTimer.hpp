#ifndef SRC_COMPONENTS_FOOTSTEPTIMER_HPP__
#define SRC_COMPONENTS_FOOTSTEPTIMER_HPP__

#include <SFML/System/Clock.hpp>

namespace Game::Cmp
{

struct FootStepTimer
{
  sf::Clock m_clock;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_FOOTSTEPTIMER_HPP__
