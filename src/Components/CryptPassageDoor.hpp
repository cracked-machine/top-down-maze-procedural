#ifndef SRC_COMPONENTS_CRYPTPASSAGEDOOR_HPP
#define SRC_COMPONENTS_CRYPTPASSAGEDOOR_HPP

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Cmp
{

enum class CryptPassageDirection { NORTH, EAST, WEST, SOUTH };

class CryptPassageDoor : public sf::Vector2f
{
public:
  CryptPassageDoor()
      : sf::Vector2f( 0.f, 0.f ),
        is_used( false )
  {
  }
  CryptPassageDoor( sf::Vector2f pos, bool used )
      : sf::Vector2f( pos ),
        is_used( used ) {};
  CryptPassageDoor( float x, float y, bool used )
      : sf::Vector2f( x, y ),
        is_used( used ) {};

  bool is_used{ false };
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_CRYPTPASSAGEDOOR_HPP