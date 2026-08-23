#ifndef SRC_COMPONENTS_CRYPT_ROOMCLOSED_HPP__
#define SRC_COMPONENTS_CRYPT_ROOMCLOSED_HPP__

#include <Components/Crypt/RoomBase.hpp>

namespace Game::Cmp::Crypt
{

//! @brief Marks a crypt room as closed: not currently accessible/carved out as part of the maze layout.
//! @note Systems::CryptSystem swaps rooms between Cmp::Crypt::RoomClosed and Cmp::Crypt::RoomOpen while
//! shuffling the maze.
class RoomClosed : public RoomBase
{
public:
  //! @brief Construct a closed room covering the given rectangle.
  //! @param bounds World-space bounds of the room.
  RoomClosed( sf::FloatRect bounds )
      : RoomBase( bounds ) {};
  //! @brief Construct a closed room covering the given rectangle.
  //! @param position Top-left world position of the room.
  //! @param size Pixel dimensions of the room.
  RoomClosed( sf::Vector2f position, sf::Vector2f size )
      : RoomBase( position, size ) {};
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_ROOMCLOSED_HPP__
