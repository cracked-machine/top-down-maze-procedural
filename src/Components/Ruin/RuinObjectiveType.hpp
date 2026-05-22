#ifndef SRC_CMP_RUIN_RUINOBJECTIVETYPE_HPP_
#define SRC_CMP_RUIN_RUINOBJECTIVETYPE_HPP_

#include <Sprites/SpriteMetaType.hpp>

namespace Game::Cmp
{

class RuinObjectiveType
{
public:
  RuinObjectiveType( Sprites::SpriteMetaType type )
      : m_type( type )
  {
  }
  Sprites::SpriteMetaType m_type;
};

} // namespace Game::Cmp

#endif // SRC_CMP_RUIN_RUINOBJECTIVETYPE_HPP_