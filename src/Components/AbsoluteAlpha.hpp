#ifndef SRC_COMPONENTS_ABSOLUTEALPHA_HPP__
#define SRC_COMPONENTS_ABSOLUTEALPHA_HPP__

#include <SFML/System/Vector2.hpp>
#include <cstdint>

namespace Game::Cmp
{

//! @brief Component to store absolute alpha value
//! Used by RenderGameSystem to set sprite alpha, this should be set by other systems
class AbsoluteAlpha
{
public:
  AbsoluteAlpha( uint8_t alpha = 255 )
      : m_alpha( alpha )
  {
  }

  uint8_t getAlpha() const { return m_alpha; }
  void setAlpha( uint8_t alpha ) { m_alpha = alpha; }

private:
  uint8_t m_alpha;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_ABSOLUTEALPHA_HPP__
