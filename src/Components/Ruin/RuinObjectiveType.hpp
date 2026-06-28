#ifndef SRC_COMPONENTS_RUIN_RUINOBJECTIVETYPE_HPP__
#define SRC_COMPONENTS_RUIN_RUINOBJECTIVETYPE_HPP__

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

#endif // SRC_COMPONENTS_RUIN_RUINOBJECTIVETYPE_HPP__
