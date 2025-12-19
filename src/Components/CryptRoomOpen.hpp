#ifndef SRC_COMPONENTS_CRYPTROOMOPEN_HPP
#define SRC_COMPONENTS_CRYPTROOMOPEN_HPP

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Cmp
{

class CryptRoomOpen : public sf::FloatRect
{
public:
  CryptRoomOpen( sf::FloatRect bounds )
      : sf::FloatRect( bounds ) {};
  CryptRoomOpen( sf::Vector2f position, sf::Vector2f size )
      : sf::FloatRect( position, size ) {};

  sf::FloatRect getBounds() const;

private:
  sf::FloatRect m_bounds;
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_CRYPTROOMOPEN_HPP