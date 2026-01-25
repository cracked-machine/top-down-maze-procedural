#ifndef __CMP_NPCSCANBOUNDS_HPP__
#define __CMP_NPCSCANBOUNDS_HPP__

#include <SFML/System/Vector2.hpp>

#include <Components/RectBounds.hpp>

namespace ProceduralMaze::Cmp {

class NPCScanBounds : public RectBounds
{
public:
  NPCScanBounds( sf::Vector2f position, sf::Vector2f size, float scale_factor )
      : RectBounds( position, size, scale_factor )
  {
  }
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP_NPCSCANBOUNDS_HPP__
