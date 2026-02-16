#ifndef SRC_COMPONENTS_RUINHEXAGRAMSEGMENT_HPP_
#define SRC_COMPONENTS_RUINHEXAGRAMSEGMENT_HPP_

#include <SFML/Graphics/RectangleShape.hpp>

#include <Components/Obstacle.hpp>

namespace ProceduralMaze::Cmp
{

// RuinMultiBlock component represents a multi-block sprite defined geometrically as a rectangle
// It can be used to find its RuinSegment parts and track overall activation state
class RuinHexagramSegment
{
public:
  RuinHexagramSegment( bool solid_mask )
      : m_solid_mask( solid_mask )
  {
  }

  bool isSolidMask() const { return m_solid_mask; }
  void set_solid_mask( bool solid_mask ) { m_solid_mask = solid_mask; }

private:
  // Is collision detection enabled for this sprite
  bool m_solid_mask{ true };
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_RUINHEXAGRAMSEGMENT_HPP_