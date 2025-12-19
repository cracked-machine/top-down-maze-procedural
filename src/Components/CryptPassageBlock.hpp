#ifndef SRC_COMPONENTS_CRYPTPASSAGEBLOCK_HPP
#define SRC_COMPONENTS_CRYPTPASSAGEBLOCK_HPP

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Cmp
{

class CryptPassageBlock : public sf::Vector2f
{
public:
  CryptPassageBlock( sf::Vector2f pos )
      : sf::Vector2f( pos ) {};
  CryptPassageBlock( float x, float y )
      : sf::Vector2f( x, y ) {};
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_CRYPTPASSAGEBLOCK_HPP