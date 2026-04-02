#ifndef SRC_CMP_CRYPT_CRYPTROOMBASE_
#define SRC_CMP_CRYPT_CRYPTROOMBASE_

#include <Crypt/CryptPassageDoor.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <Utils.hpp>

namespace ProceduralMaze::Cmp
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
    for ( auto &midpoint : m_passageconnections )
    {
      midpoint.second.is_used = used;
    }
  }

  bool are_all_doors_used()
  {
    bool result = true;
    for ( auto &midpoint : m_passageconnections )
    {
      if ( not midpoint.second.is_used ) result = false;
    }
    return result;
  }

  void init_passage_connection( bool north, bool south, bool west, bool east )
  {
    float half_width = size.x / 2;
    float half_height = size.y / 2;

    // clang-format off
    if(north) {
      m_passageconnections[CryptPassageDirection::NORTH] = 
        CryptPassageDoor(Utils::snap_to_grid( sf::Vector2f{ position.x + half_width, position.y - Constants::kGridSizePxF.y },
                                              Utils::Rounding::TOWARDS_ZERO ), 
                                              false, 
                                              CryptPassageDirection::NORTH );
    }
    if( south) {
      m_passageconnections[CryptPassageDirection::EAST] = 
        CryptPassageDoor(Utils::snap_to_grid( sf::Vector2f{ position.x + size.x, position.y + half_height }, 
                                              Utils::Rounding::TOWARDS_ZERO ), 
                                              false,
                                              CryptPassageDirection::EAST );
    }
    if(west) {
      m_passageconnections[CryptPassageDirection::WEST] = 
        CryptPassageDoor(Utils::snap_to_grid( sf::Vector2f{ position.x - Constants::kGridSizePxF.x, position.y + half_height },
                                              Utils::Rounding::TOWARDS_ZERO ),
                                              false, 
                                              CryptPassageDirection::WEST );
    }
    if(east) {
      m_passageconnections[CryptPassageDirection::SOUTH] = 
        CryptPassageDoor(Utils::snap_to_grid( sf::Vector2f( position.x + half_width, position.y + size.y ), 
                                              Utils::Rounding::TOWARDS_ZERO ), 
                                              false,
                                              CryptPassageDirection::SOUTH );
    }
    // clang-format on   
    
  }

  std::vector<std::pair<entt::entity, Cmp::Position>> m_position_list;
  std::vector<std::pair<entt::entity, Cmp::Position>> m_border_position_list;
  std::unordered_map<CryptPassageDirection, CryptPassageDoor> m_passageconnections;
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMP_CRYPT_CRYPTROOMBASE_