#ifndef SRC_CMPS_NPCSHOCKWAVE_
#define SRC_CMPS_NPCSHOCKWAVE_

#include <SFML/Graphics/CircleShape.hpp>
#include <Utils/Utils.hpp>

namespace ProceduralMaze::Cmp
{

class NpcShockwave : public sf::CircleShape
{
public:
  NpcShockwave( sf::Vector2f pos )
      : CircleShape( Constants::kGridSquareSizePixelsF.x ) // set radius
  {
    setPosition( pos );
    setFillColor( sf::Color::Transparent );
    setOutlineColor( sf::Color::Red );
    setOutlineThickness( 2.f );
  }
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_NPCSHOCKWAVE_