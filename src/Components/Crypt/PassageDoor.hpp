#ifndef SRC_COMPONENTS_CRYPT_PASSAGEDOOR_HPP__
#define SRC_COMPONENTS_CRYPT_PASSAGEDOOR_HPP__

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace Game::Cmp::Crypt
{

enum class CryptPassageDirection { NORTH, EAST, WEST, SOUTH };

class PassageDoor : public sf::Vector2f
{
public:
  //! @brief default constructor for std::unordered_map
  PassageDoor()
      : sf::Vector2f( 0.f, 0.f ),
        is_used( false )
  {
  }
  PassageDoor( sf::Vector2f pos, bool used, CryptPassageDirection direction )
      : sf::Vector2f( pos ),
        is_used( used ),
        m_direction( direction ) {};
  PassageDoor( float x, float y, bool used, CryptPassageDirection direction )
      : sf::Vector2f( x, y ),
        is_used( used ),
        m_direction( direction ) {};

  bool is_used{ false };
  CryptPassageDirection m_direction;
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_PASSAGEDOOR_HPP__
