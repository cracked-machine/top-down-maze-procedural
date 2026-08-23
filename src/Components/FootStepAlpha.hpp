#ifndef SRC_COMPONENTS_FOOTSTEPALPHA_HPP__
#define SRC_COMPONENTS_FOOTSTEPALPHA_HPP__

#include <SFML/System/Clock.hpp>

namespace Game::Cmp
{

//! @brief Fade-out alpha for a footstep mark entity; decremented over time until it reaches
//!        zero, at which point the footstep entity is destroyed.
struct FootStepAlpha
{
  //! @brief Current opacity of the footstep mark, from 255 (fully visible) to 0 (invisible).
  uint8_t m_alpha{ 255 };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_FOOTSTEPALPHA_HPP__
