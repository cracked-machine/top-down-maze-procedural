#ifndef SRC_COMPONENTS_CRYPTROOMLAVAPITCELL_HPP
#define SRC_COMPONENTS_CRYPTROOMLAVAPITCELL_HPP

#include <Components/Position.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Cmp
{

class CryptRoomLavaPitCell : public Cmp::Position
{
public:
  CryptRoomLavaPitCell( const sf::Vector2f &pos, const sf::Vector2f &size )
      : Position( pos, size )
  {
  }
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_CRYPTROOMLAVAPITCELL_HPP