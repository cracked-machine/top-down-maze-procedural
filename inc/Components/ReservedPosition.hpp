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
  ReservedPosition( Cmp::Position pos, bool solid_mask, Sprites::SpriteMetaType type, unsigned int sprite_index )
      : Cmp::Position( pos ),
        m_solid_mask( solid_mask ),
        m_type( type ),
        m_sprite_index( sprite_index )
  {
  }

  // Is collision detection enabled for this sprite
  bool m_solid_mask{ true };

  // The type of sprite to use from the SpriteFactory
  Sprites::SpriteMetaType m_type;

  // The sprite index to use from the MultiSprite associated with 'm_type'
  unsigned int m_sprite_index{ 0 };

  // this is a one way switch - once animated, it cannot be disabled
  void animate() { m_animated = true; }
  bool is_animated() const { return m_animated; }

  void break_object() { m_broken = true; }
  bool is_broken() const { return m_broken; }

private:
  bool m_animated{ false };
  bool m_broken{ false };
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP_RESERVED_POSITION_HPP__