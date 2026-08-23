#ifndef SRC_COMPONENTS_CRYPT_PASSAGEBLOCK_HPP__
#define SRC_COMPONENTS_CRYPT_PASSAGEBLOCK_HPP__

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace Game::Cmp::Crypt
{

//! @brief Marks a single floor tile of a corridor connecting two crypt rooms, tagged with the id of the
//! passage it belongs to.
class PassageBlock : public sf::Vector2f
{
public:
  //! @brief Construct a new passage block at the given position.
  //! @param pos World position of the block.
  //! @param passage_id Id of the passage this block belongs to.
  PassageBlock( sf::Vector2f pos, unsigned int passage_id )
      : sf::Vector2f( pos ),
        m_passage_id( passage_id ) {};
  //! @brief Construct a new passage block at the given position.
  //! @param x World x position of the block.
  //! @param y World y position of the block.
  //! @param passage_id Id of the passage this block belongs to.
  PassageBlock( float x, float y, unsigned int passage_id )
      : sf::Vector2f( x, y ),
        m_passage_id( passage_id ) {};

  //! @brief Id of the passage this block belongs to.
  unsigned int m_passage_id = 0;
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_PASSAGEBLOCK_HPP__
