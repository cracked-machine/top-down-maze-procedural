#ifndef SRC_COMPONENTS_FOOTSTEPTIMER_HPP__
#define SRC_COMPONENTS_FOOTSTEPTIMER_HPP__

#include <SFML/System/Clock.hpp>

namespace Game::Cmp
{

//! @brief Tracks how long a footstep mark entity has existed, used to decide when it should
//!        start fading (see Cmp::FootStepAlpha).
struct FootStepTimer
{
  //! @brief Elapsed-time clock started when the footstep mark was created.
  sf::Clock m_clock;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_FOOTSTEPTIMER_HPP__
