#ifndef SRC_COMPONENTS_CRYPTROOMCLOSED_HPP
#define SRC_COMPONENTS_CRYPTROOMCLOSED_HPP

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Cmp
{

class CryptRoomClosed : public sf::FloatRect
{
public:
  CryptRoomClosed( sf::FloatRect bounds )
      : sf::FloatRect( bounds ) {};
  CryptRoomClosed( sf::Vector2f position, sf::Vector2f size )
      : sf::FloatRect( position, size ) {};

  sf::FloatRect getBounds() const;

private:
  sf::FloatRect m_bounds;
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_CRYPTROOMCLOSED_HPP