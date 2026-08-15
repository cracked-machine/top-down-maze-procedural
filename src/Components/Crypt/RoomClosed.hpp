#ifndef SRC_COMPONENTS_CRYPT_ROOMCLOSED_HPP__
#define SRC_COMPONENTS_CRYPT_ROOMCLOSED_HPP__

#include <Components/Crypt/RoomBase.hpp>

namespace Game::Cmp::Crypt
{

class RoomClosed : public RoomBase
{
public:
  RoomClosed( sf::FloatRect bounds )
      : RoomBase( bounds ) {};
  RoomClosed( sf::Vector2f position, sf::Vector2f size )
      : RoomBase( position, size ) {};
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_ROOMCLOSED_HPP__
