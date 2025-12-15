#ifndef CRYPT_ROOM_HPP
#define CRYPT_ROOM_HPP

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Cmp
{

class CryptRoom : public sf::FloatRect
{
public:
  CryptRoom( sf::FloatRect bounds )
      : sf::FloatRect( bounds ) {};
  CryptRoom( sf::Vector2f position, sf::Vector2f size )
      : sf::FloatRect( position, size ) {};

  sf::FloatRect getBounds() const;

private:
  sf::FloatRect m_bounds;
};

} // namespace ProceduralMaze::Cmp

#endif // CRYPT_ROOM_HPP