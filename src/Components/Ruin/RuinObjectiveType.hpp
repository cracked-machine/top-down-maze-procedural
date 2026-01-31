#ifndef SRC_CMP_RUIN_RUINOBJECTIVETYPE_HPP_
#define SRC_CMP_RUIN_RUINOBJECTIVETYPE_HPP_

#include <Sprites/MultiSprite.hpp>

namespace ProceduralMaze::Cmp
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

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMP_RUIN_RUINOBJECTIVETYPE_HPP_