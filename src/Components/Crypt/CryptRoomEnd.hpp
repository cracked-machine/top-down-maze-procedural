#ifndef SRC_COMPONENTS_CRYPT_CRYPTROOMEND_HPP__
#define SRC_COMPONENTS_CRYPT_CRYPTROOMEND_HPP__

#include <Components/Crypt/CryptPassageDoor.hpp>
#include <Components/Crypt/CryptRoomBase.hpp>
#include <SFML/Graphics/Rect.hpp>
namespace Game::Cmp
{

class CryptRoomEnd : public CryptRoomBase
{
public:
  CryptRoomEnd( sf::FloatRect bounds )
      : CryptRoomBase( bounds )
  {
    init_passage_connection( false, true, true, true );
  };
  CryptRoomEnd( sf::Vector2f position, sf::Vector2f size )
      : CryptRoomBase( position, size )
  {
    init_passage_connection( false, true, true, true );
  };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_CRYPT_CRYPTROOMEND_HPP__
