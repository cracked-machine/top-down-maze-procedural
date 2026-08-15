#ifndef SRC_COMPONENTS_CRYPT_ROOMSTART_HPP__
#define SRC_COMPONENTS_CRYPT_ROOMSTART_HPP__

#include <Components/Crypt/RoomBase.hpp>

namespace Game::Cmp::Crypt
{

class RoomStart : public RoomBase
{
public:
  RoomStart( sf::FloatRect bounds )
      : RoomBase( bounds )
  {
    init_passage_connection( true, false, true, true );
  };
  RoomStart( sf::Vector2f position, sf::Vector2f size )
      : RoomBase( position, size )
  {
    init_passage_connection( true, false, true, true );
  };
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_ROOMSTART_HPP__
