#ifndef SRC_COMPONENTS_CRYPT_CRYPTROOMSTART_HPP__
#define SRC_COMPONENTS_CRYPT_CRYPTROOMSTART_HPP__

#include <Components/Crypt/CryptRoomBase.hpp>

namespace Game::Cmp
{

class CryptRoomStart : public CryptRoomBase
{
public:
  CryptRoomStart( sf::FloatRect bounds )
      : CryptRoomBase( bounds )
  {
    init_passage_connection( true, false, true, true );
  };
  CryptRoomStart( sf::Vector2f position, sf::Vector2f size )
      : CryptRoomBase( position, size )
  {
    init_passage_connection( true, false, true, true );
  };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_CRYPT_CRYPTROOMSTART_HPP__
