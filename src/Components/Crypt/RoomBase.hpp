#ifndef SRC_COMPONENTS_CRYPT_ROOMBASE_HPP__
#define SRC_COMPONENTS_CRYPT_ROOMBASE_HPP__

#include <Components/Crypt/PassageDoor.hpp>
#include <Components/Position.hpp>
#include <SFML/Graphics/Rect.hpp>

namespace Cmp
{
class Position;
} // namespace Cmp

namespace Game::Cmp::Crypt
{

//! @brief Base geometry and passage-connector state shared by all crypt room types, defined geometrically
//! as a rectangle.
//! @note Concrete room types (Cmp::Crypt::RoomStart, Cmp::Crypt::RoomEnd, Cmp::Crypt::RoomOpen,
//! Cmp::Crypt::RoomClosed) derive from this to select which sides get a Cmp::Crypt::PassageDoor connector.
class RoomBase : public sf::FloatRect
{
public:
  //! @brief Construct a room covering the given rectangle with a passage connector on every side.
  //! @param bounds World-space bounds of the room.
  RoomBase( sf::FloatRect bounds )
      : sf::FloatRect( bounds )
  {
    init_passage_connection( true, true, true, true );
  };

  //! @brief Construct a room covering the given rectangle with a passage connector on every side.
  //! @param position Top-left world position of the room.
  //! @param size Pixel dimensions of the room.
  RoomBase( sf::Vector2f position, sf::Vector2f size )
      : sf::FloatRect( position, size )
  {
    init_passage_connection( true, true, true, true );
  };

  //! @brief Set the used/connected state of every passage door connector on this room.
  //! @param used
  void set_all_doors_used( bool used )
  {
    for ( auto &midpoint : m_connectors )
    {
      midpoint.second.is_used = used;
    }
  }

  //! @brief Check whether every passage door connector on this room is marked as used/connected.
  //! @return bool
  bool are_all_doors_used()
  {
    bool result = true;
    for ( auto &midpoint : m_connectors )
    {
      if ( not midpoint.second.is_used ) result = false;
    }
    return result;
  }

  //! @brief Populate Cmp::Crypt::PassageDoor connectors for the requested sides of this room.
  //! @param north Whether to create a connector on the north side.
  //! @param south Whether to create a connector on the south side.
  //! @param west Whether to create a connector on the west side.
  //! @param east Whether to create a connector on the east side.
  void init_passage_connection( bool north, bool south, bool west, bool east );

  //! @brief Entities and positions of the room's interior floor tiles.
  std::vector<std::pair<entt::entity, Cmp::Position>> m_position_list;
  //! @brief Entities and positions of the room's border/wall tiles.
  std::vector<std::pair<entt::entity, Cmp::Position>> m_border_position_list;
  //! @brief Passage door connectors on this room, keyed by which side they sit on.
  std::unordered_map<CryptPassageDirection, PassageDoor> m_connectors;
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_ROOMBASE_HPP__
