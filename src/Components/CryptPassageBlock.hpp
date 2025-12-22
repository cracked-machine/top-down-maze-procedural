#ifndef SRC_COMPONENTS_CRYPTPASSAGEBLOCK_HPP
#define SRC_COMPONENTS_CRYPTPASSAGEBLOCK_HPP

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Cmp
{

class CryptPassageBlock : public sf::Vector2f
{
public:
  CryptPassageBlock( sf::Vector2f pos, unsigned int passage_id )
      : sf::Vector2f( pos ),
        m_passage_id( passage_id ) {};
  CryptPassageBlock( float x, float y, unsigned int passage_id )
      : sf::Vector2f( x, y ),
        m_passage_id( passage_id ) {};

  unsigned int m_passage_id = 0;
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_CRYPTPASSAGEBLOCK_HPP