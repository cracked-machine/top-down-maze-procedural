#ifndef SRC_COMPONENTS_CRYPT_CRYPTROOMLAVAPIT_HPP__
#define SRC_COMPONENTS_CRYPT_CRYPTROOMLAVAPIT_HPP__

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace Game::Cmp
{

class CryptRoomLavaPit : public sf::FloatRect
{
public:
  CryptRoomLavaPit( sf::FloatRect bounds )
      : sf::FloatRect( bounds ) {

        };
  CryptRoomLavaPit( sf::Vector2f position, sf::Vector2f size )
      : sf::FloatRect( position, size ) {

        };

  sf::FloatRect getBounds() const;

private:
  sf::FloatRect m_bounds;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_CRYPT_CRYPTROOMLAVAPIT_HPP__
