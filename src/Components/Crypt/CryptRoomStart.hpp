#ifndef SRC_COMPONENTS_CRYPTROOMSTART_HPP
#define SRC_COMPONENTS_CRYPTROOMSTART_HPP

#include <Components/Crypt/CryptPassageDoor.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <Utils/Utils.hpp>
#include <unordered_map>

namespace ProceduralMaze::Cmp
{

class CryptRoomStart : public sf::FloatRect
{
public:
  CryptRoomStart( sf::FloatRect bounds )
      : sf::FloatRect( bounds )
  {
    init_mid_points();
  };
  CryptRoomStart( sf::Vector2f position, sf::Vector2f size )
      : sf::FloatRect( position, size )
  {
    init_mid_points();
  };

  sf::FloatRect getBounds() const;

  void set_all_doors_used( bool used )
  {
    for ( auto &midpoint : m_midpoints )
    {
      midpoint.second.is_used = used;
    }
  }

  void init_mid_points()
  {
    float half_width = size.x / 2;
    float half_height = size.y / 2;

    // clang-format off
    m_midpoints[CryptPassageDirection::NORTH] = 
      CryptPassageDoor(Utils::snap_to_grid( sf::Vector2f{ position.x + half_width, position.y - Constants::kGridSizePxF.y },
                                            Utils::Rounding::TOWARDS_ZERO ), 
                                            false, 
                                            CryptPassageDirection::NORTH );

    m_midpoints[CryptPassageDirection::EAST] = 
      CryptPassageDoor(Utils::snap_to_grid( sf::Vector2f{ position.x + size.x, position.y + half_height }, 
                                            Utils::Rounding::TOWARDS_ZERO ), 
                                            false,
                                            CryptPassageDirection::EAST );

    m_midpoints[CryptPassageDirection::WEST] = 
      CryptPassageDoor(Utils::snap_to_grid( sf::Vector2f{ position.x - Constants::kGridSizePxF.x, position.y + half_height },
                                            Utils::Rounding::TOWARDS_ZERO ),
                                            false, 
                                            CryptPassageDirection::WEST );

    // clang-format on  
  }

  std::unordered_map<CryptPassageDirection, CryptPassageDoor> m_midpoints;

private:
  sf::FloatRect m_bounds;
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_CRYPTROOMSTART_HPP