#ifndef __CMP_NPCSCANBOUNDS_HPP__
#define __CMP_NPCSCANBOUNDS_HPP__

#include <RectBounds.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

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
