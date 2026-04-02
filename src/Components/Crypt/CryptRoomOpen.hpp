#ifndef SRC_COMPONENTS_CRYPTROOMOPEN_HPP
#define SRC_COMPONENTS_CRYPTROOMOPEN_HPP

#include <Crypt/CryptRoomBase.hpp>

namespace ProceduralMaze::Cmp
{

class CryptRoomOpen : public CryptRoomBase
{
public:
  CryptRoomOpen( sf::FloatRect bounds )
      : CryptRoomBase( bounds ) {};
  CryptRoomOpen( sf::Vector2f position, sf::Vector2f size )
      : CryptRoomBase( position, size ) {};
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_CRYPTROOMOPEN_HPP