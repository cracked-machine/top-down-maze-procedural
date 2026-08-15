#ifndef SRC_COMPONENTS_RUIN_OBJECTIVETYPE_HPP__
#define SRC_COMPONENTS_RUIN_OBJECTIVETYPE_HPP__

#include <Sprites/SpriteMetaType.hpp>

namespace Game::Cmp::Ruin
{

class ObjectiveType
{
public:
  ObjectiveType( Sprites::SpriteMetaType type )
      : m_type( type )
  {
  }
  Sprites::SpriteMetaType m_type;
};

} // namespace Game::Cmp::Ruin

#endif // SRC_COMPONENTS_RUIN_OBJECTIVETYPE_HPP__
