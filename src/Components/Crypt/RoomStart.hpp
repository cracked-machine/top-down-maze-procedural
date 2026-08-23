#ifndef SRC_COMPONENTS_CRYPT_ROOMSTART_HPP__
#define SRC_COMPONENTS_CRYPT_ROOMSTART_HPP__

#include <Components/Crypt/RoomBase.hpp>

namespace Game::Cmp::Crypt
{

//! @brief Marks the single entry room of the crypt maze where the player begins.
//! @note Systems::CryptSystem::get_crypt_room_start() looks up the one entity carrying this component.
class RoomStart : public RoomBase
{
public:
  //! @brief Construct the start room covering the given rectangle.
  //! @param bounds World-space bounds of the room.
  RoomStart( sf::FloatRect bounds )
      : RoomBase( bounds )
  {
    init_passage_connection( true, false, true, true );
  };
  //! @brief Construct the start room covering the given rectangle.
  //! @param position Top-left world position of the room.
  //! @param size Pixel dimensions of the room.
  RoomStart( sf::Vector2f position, sf::Vector2f size )
      : RoomBase( position, size )
  {
    init_passage_connection( true, false, true, true );
  };
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_ROOMSTART_HPP__
