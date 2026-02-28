#ifndef SRC_CMP_NPC_NPCPATHTRAJECTORY_HPP
#define SRC_CMP_NPC_NPCPATHTRAJECTORY_HPP

#include <SFML/Graphics/Rect.hpp>
namespace ProceduralMaze::Cmp
{

class NpcPathTrajectory : public sf::FloatRect
{
public:
  NpcPathTrajectory( sf::FloatRect rect )
      : sf::FloatRect( rect.position, rect.size )
  {
  }
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMP_NPC_NPCPATHTRAJECTORY_HPP