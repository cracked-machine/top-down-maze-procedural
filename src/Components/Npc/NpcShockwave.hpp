#ifndef SRC_CMPS_NPCSHOCKWAVE_
#define SRC_CMPS_NPCSHOCKWAVE_

#include <Sprites/Shockwave.hpp>

namespace ProceduralMaze::Cmp
{

class NpcShockwave
{
public:
  NpcShockwave( sf::Vector2f pos, int points_per_segment )
      : sprite( pos, points_per_segment )
  {
  }

  Sprites::Shockwave sprite;
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_NPCSHOCKWAVE_