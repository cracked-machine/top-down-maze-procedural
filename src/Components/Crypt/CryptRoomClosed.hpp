#ifndef SRC_COMPONENTS_CRYPTROOMCLOSED_HPP
#define SRC_COMPONENTS_CRYPTROOMCLOSED_HPP

#include <Crypt/CryptRoomBase.hpp>

namespace ProceduralMaze::Cmp
{

class CryptRoomClosed : public CryptRoomBase
{
public:
  CryptRoomClosed( sf::FloatRect bounds )
      : CryptRoomBase( bounds ) {};
  CryptRoomClosed( sf::Vector2f position, sf::Vector2f size )
      : CryptRoomBase( position, size ) {};
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_CRYPTROOMCLOSED_HPP