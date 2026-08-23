#ifndef SRC_COMPONENTS_CRYPT_ROOMOPEN_HPP__
#define SRC_COMPONENTS_CRYPT_ROOMOPEN_HPP__

#include <Components/Crypt/RoomBase.hpp>

namespace Game::Cmp::Crypt
{

//! @brief Marks a crypt room as open: currently accessible/carved out as part of the maze layout.
//! @note Systems::CryptSystem swaps rooms between Cmp::Crypt::RoomOpen and Cmp::Crypt::RoomClosed while
//! shuffling the maze, and hosts levers, lava pits and other room content while open.
class RoomOpen : public RoomBase
{
public:
  //! @brief Construct an open room covering the given rectangle.
  //! @param bounds World-space bounds of the room.
  RoomOpen( sf::FloatRect bounds )
      : RoomBase( bounds ) {};
  //! @brief Construct an open room covering the given rectangle.
  //! @param position Top-left world position of the room.
  //! @param size Pixel dimensions of the room.
  RoomOpen( sf::Vector2f position, sf::Vector2f size )
      : RoomBase( position, size ) {};
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_ROOMOPEN_HPP__
