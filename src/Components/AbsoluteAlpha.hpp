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
  //! @brief Construct with an initial alpha value.
  //! @param alpha Alpha value in the range [0, 255]. Defaults to fully opaque.
  AbsoluteAlpha( uint8_t alpha = 255 )
      : m_alpha( alpha )
  {
  }

  //! @brief Get the stored alpha value.
  //! @return uint8_t Alpha value in the range [0, 255].
  uint8_t getAlpha() const { return m_alpha; }

  //! @brief Set the stored alpha value.
  //! @param alpha Alpha value in the range [0, 255].
  void setAlpha( uint8_t alpha ) { m_alpha = alpha; }

private:
  //! @brief Alpha value in the range [0, 255].
  uint8_t m_alpha;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_ABSOLUTEALPHA_HPP__
