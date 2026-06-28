#ifndef SRC_COMPONENTS_CRYPTROOMOPEN_HPP
#define SRC_COMPONENTS_CRYPTROOMOPEN_HPP

#include <Components/Crypt/CryptRoomBase.hpp>

namespace Game::Cmp
{

class CryptRoomOpen : public CryptRoomBase
{
public:
  CryptRoomOpen( sf::FloatRect bounds )
      : CryptRoomBase( bounds ) {};
  CryptRoomOpen( sf::Vector2f position, sf::Vector2f size )
      : CryptRoomBase( position, size ) {};
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_CRYPTROOMOPEN_HPP