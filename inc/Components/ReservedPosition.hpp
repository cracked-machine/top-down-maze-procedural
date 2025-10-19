#ifndef __CMP_RESERVED_POSITION_HPP__
#define __CMP_RESERVED_POSITION_HPP__

#include <Position.hpp>
#include <SFML/System/Vector2.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <entt/entity/fwd.hpp>

namespace ProceduralMaze::Cmp {

class ReservedPosition : public Cmp::Position
{
public:
  ReservedPosition( sf::Vector2f pos, bool solid_mask, Sprites::SpriteMetaType type, unsigned int sprite_index )
      : Cmp::Position( pos ),
        m_solid_mask( solid_mask ),
        m_type( type ),
        m_sprite_index( sprite_index )
  {
  }
  bool m_solid_mask{ true };
  Sprites::SpriteMetaType m_type;
  unsigned int m_sprite_index{ 0 };
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP_RESERVED_POSITION_HPP__