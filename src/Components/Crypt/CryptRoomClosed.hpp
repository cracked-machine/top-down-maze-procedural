#ifndef SRC_COMPONENTS_CRYPT_CRYPTROOMCLOSED_HPP__
#define SRC_COMPONENTS_CRYPT_CRYPTROOMCLOSED_HPP__

#include <Components/Crypt/CryptRoomBase.hpp>

namespace Game::Cmp
{

class CryptRoomClosed : public CryptRoomBase
{
public:
  CryptRoomClosed( sf::FloatRect bounds )
      : CryptRoomBase( bounds ) {};
  CryptRoomClosed( sf::Vector2f position, sf::Vector2f size )
      : CryptRoomBase( position, size ) {};
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_CRYPT_CRYPTROOMCLOSED_HPP__
