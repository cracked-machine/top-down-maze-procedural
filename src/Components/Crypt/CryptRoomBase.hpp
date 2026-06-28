#ifndef SRC_COMPONENTS_CRYPT_CRYPTROOMBASE_HPP__
#define SRC_COMPONENTS_CRYPT_CRYPTROOMBASE_HPP__

#include <Components/Crypt/CryptPassageDoor.hpp>
#include <Components/Position.hpp>
#include <SFML/Graphics/Rect.hpp>

namespace Cmp
{
class Position;
} // namespace Cmp

namespace Game::Cmp
{

class CryptRoomBase : public sf::FloatRect
{
public:
  CryptRoomBase( sf::FloatRect bounds )
      : sf::FloatRect( bounds )
  {
    init_passage_connection( true, true, true, true );
  };

  CryptRoomBase( sf::Vector2f position, sf::Vector2f size )
      : sf::FloatRect( position, size )
  {
    init_passage_connection( true, true, true, true );
  };

  void set_all_doors_used( bool used )
  {
    for ( auto &midpoint : m_connectors )
    {
      midpoint.second.is_used = used;
    }
  }

  bool are_all_doors_used()
  {
    bool result = true;
    for ( auto &midpoint : m_connectors )
    {
      if ( not midpoint.second.is_used ) result = false;
    }
    return result;
  }

  void init_passage_connection( bool north, bool south, bool west, bool east );

  std::vector<std::pair<entt::entity, Cmp::Position>> m_position_list;
  std::vector<std::pair<entt::entity, Cmp::Position>> m_border_position_list;
  std::unordered_map<CryptPassageDirection, CryptPassageDoor> m_connectors;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_CRYPT_CRYPTROOMBASE_HPP__
