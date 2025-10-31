#ifndef __CMP_RESERVED_POSITION_HPP__
#define __CMP_RESERVED_POSITION_HPP__

#include <entt/entity/fwd.hpp>

#include <SFML/System/Vector2.hpp>

#include <Components/Position.hpp>
#include <Sprites/SpriteFactory.hpp>

namespace ProceduralMaze::Cmp {

class ReservedPosition
{
public:
  ReservedPosition() {}

  bool reserved{ true };
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP_RESERVED_POSITION_HPP__