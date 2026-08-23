#ifndef SRC_COMPONENTS_ABSOLUTEROTATION_HPP__
#define SRC_COMPONENTS_ABSOLUTEROTATION_HPP__

#include <SFML/System/Vector2.hpp>

namespace Game::Cmp
{

//! @brief Component to store absolute rotation angle in degrees
//! Used by RenderGameSystem to rotate sprites, this should be set by other systems
class AbsoluteRotation
{
public:
  //! @brief Construct with an initial rotation angle.
  //! @param angle Rotation angle in degrees. Defaults to zero.
  AbsoluteRotation( float angle = 0.f )
      : m_angle( angle )
  {
  }

  //! @brief Get the stored rotation angle.
  //! @return float Rotation angle in degrees.
  float getAngle() const { return m_angle; }

  //! @brief Set the stored rotation angle.
  //! @param angle Rotation angle in degrees.
  void setAngle( float angle ) { m_angle = angle; }

private:
  //! @brief Rotation angle in degrees.
  float m_angle;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_ABSOLUTEROTATION_HPP__
