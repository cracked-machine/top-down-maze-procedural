#ifndef SRC_COMPONENTS_CRYPT_ROOMEND_HPP__
#define SRC_COMPONENTS_CRYPT_ROOMEND_HPP__

#include <Components/Crypt/PassageDoor.hpp>
#include <Components/Crypt/RoomBase.hpp>
#include <SFML/Graphics/Rect.hpp>
namespace Game::Cmp::Crypt
{

class RoomEnd : public RoomBase
{
public:
  RoomEnd( sf::FloatRect bounds )
      : RoomBase( bounds )
  {
    init_passage_connection( false, true, true, true );
  };
  RoomEnd( sf::Vector2f position, sf::Vector2f size )
      : RoomBase( position, size )
  {
    init_passage_connection( false, true, true, true );
  };
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_ROOMEND_HPP__
