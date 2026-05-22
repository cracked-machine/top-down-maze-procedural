#ifndef __CMP_RESERVED_POSITION_HPP__
#define __CMP_RESERVED_POSITION_HPP__

#include <entt/entity/fwd.hpp>

#include <SFML/System/Vector2.hpp>

#include <Components/Position.hpp>
#include <Factory/SpriteFactory.hpp>

namespace Game::Cmp
{

//! @brief Used to prevent algorithmic changes to this entities components.
class ReservedPosition
{
public:
  ReservedPosition() {}

  bool reserved{ true };
};

} // namespace Game::Cmp

#endif // __CMP_RESERVED_POSITION_HPP__