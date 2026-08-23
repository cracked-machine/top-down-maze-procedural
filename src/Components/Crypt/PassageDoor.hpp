#ifndef SRC_COMPONENTS_CRYPT_PASSAGEDOOR_HPP__
#define SRC_COMPONENTS_CRYPT_PASSAGEDOOR_HPP__

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace Game::Cmp::Crypt
{

//! @brief Compass side of a Cmp::Crypt::RoomBase that a Cmp::Crypt::PassageDoor connector sits on.
enum class CryptPassageDirection
{
  //! @brief Door on the room's north side.
  NORTH,
  //! @brief Door on the room's east side.
  EAST,
  //! @brief Door on the room's west side.
  WEST,
  //! @brief Door on the room's south side.
  SOUTH
};

//! @brief A connector point on the edge of a Cmp::Crypt::RoomBase where a passage/corridor may join to a
//! neighboring room.
class PassageDoor : public sf::Vector2f
{
public:
  //! @brief default constructor for std::unordered_map
  PassageDoor()
      : sf::Vector2f( 0.f, 0.f ),
        is_used( false )
  {
  }
  //! @brief Construct a new passage door.
  //! @param pos World position of the door.
  //! @param used Whether this door is already connected/consumed.
  //! @param direction Compass side of the owning room this door sits on.
  PassageDoor( sf::Vector2f pos, bool used, CryptPassageDirection direction )
      : sf::Vector2f( pos ),
        is_used( used ),
        m_direction( direction ) {};
  //! @brief Construct a new passage door.
  //! @param x World x position of the door.
  //! @param y World y position of the door.
  //! @param used Whether this door is already connected/consumed.
  //! @param direction Compass side of the owning room this door sits on.
  PassageDoor( float x, float y, bool used, CryptPassageDirection direction )
      : sf::Vector2f( x, y ),
        is_used( used ),
        m_direction( direction ) {};

  //! @brief Whether this door has already been connected to a neighboring room's passage.
  bool is_used{ false };
  //! @brief Compass side of the owning room this door sits on.
  CryptPassageDirection m_direction;
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_PASSAGEDOOR_HPP__
