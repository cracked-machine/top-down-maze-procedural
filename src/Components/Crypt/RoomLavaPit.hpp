#ifndef SRC_COMPONENTS_CRYPT_ROOMLAVAPIT_HPP__
#define SRC_COMPONENTS_CRYPT_ROOMLAVAPIT_HPP__

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace Game::Cmp::Crypt
{

class RoomLavaPit : public sf::FloatRect
{
public:
  RoomLavaPit( sf::FloatRect bounds )
      : sf::FloatRect( bounds ) {

        };
  RoomLavaPit( sf::Vector2f position, sf::Vector2f size )
      : sf::FloatRect( position, size ) {

        };

  sf::FloatRect getBounds() const;

private:
  sf::FloatRect m_bounds;
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_ROOMLAVAPIT_HPP__
