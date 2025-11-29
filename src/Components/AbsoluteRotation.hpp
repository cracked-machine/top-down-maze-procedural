#ifndef INC_COMPONENTS_ABSOLUTEROTATION_HPP
#define INC_COMPONENTS_ABSOLUTEROTATION_HPP

#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Cmp
{

//! @brief Component to store absolute rotation angle in degrees
//! Used by RenderGameSystem to rotate sprites, this should be set by other systems
class AbsoluteRotation
{
public:
  AbsoluteRotation( float angle = 0.f )
      : m_angle( angle )
  {
  }

  float getAngle() const { return m_angle; }
  void setAngle( float angle ) { m_angle = angle; }

private:
  float m_angle;
};

} // namespace ProceduralMaze::Cmp

#endif // INC_COMPONENTS_ABSOLUTEROTATION_HPP