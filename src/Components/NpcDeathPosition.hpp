#ifndef __CMP_NPC_DEATH_POSITION_HPP__
#define __CMP_NPC_DEATH_POSITION_HPP__

#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Cmp {

class NpcDeathPosition : public sf::Vector2f
{
public:
  NpcDeathPosition( sf::Vector2f pos )
      : sf::Vector2f( pos )
  {
  }
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP_NPC_DEATH_POSITION_HPP__