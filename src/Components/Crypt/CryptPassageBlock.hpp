#ifndef SRC_COMPONENTS_CRYPT_CRYPTPASSAGEBLOCK_HPP__
#define SRC_COMPONENTS_CRYPT_CRYPTPASSAGEBLOCK_HPP__

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace Game::Cmp
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

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_CRYPT_CRYPTPASSAGEBLOCK_HPP__
