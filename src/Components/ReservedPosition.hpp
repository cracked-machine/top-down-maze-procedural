#ifndef SRC_COMPONENTS_RESERVEDPOSITION_HPP__
#define SRC_COMPONENTS_RESERVEDPOSITION_HPP__

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
  //! @brief Default construct a reserved marker.
  ReservedPosition() {}

  //! @brief Whether this entity's position is currently reserved from algorithmic changes.
  bool reserved{ true };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_RESERVEDPOSITION_HPP__
