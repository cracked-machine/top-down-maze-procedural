#ifndef SRC_COMPONENTS_CRYPTROOMEND_HPP
#define SRC_COMPONENTS_CRYPTROOMEND_HPP

#include <Components/Crypt/CryptPassageDoor.hpp>
#include <Components/Crypt/CryptRoomBase.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <Utils/Utils.hpp>

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

#endif // SRC_COMPONENTS_CRYPTROOMEND_HPP