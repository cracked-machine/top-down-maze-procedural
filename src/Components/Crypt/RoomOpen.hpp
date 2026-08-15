#ifndef SRC_COMPONENTS_CRYPT_ROOMOPEN_HPP__
#define SRC_COMPONENTS_CRYPT_ROOMOPEN_HPP__

#include <Components/Crypt/RoomBase.hpp>

namespace Game::Cmp::Crypt
{

class RoomOpen : public RoomBase
{
public:
  RoomOpen( sf::FloatRect bounds )
      : RoomBase( bounds ) {};
  RoomOpen( sf::Vector2f position, sf::Vector2f size )
      : RoomBase( position, size ) {};
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_ROOMOPEN_HPP__
