#ifndef SRC_COMPONENTS_CRYPT_ROOMLAVAPITCELLEFFECT_HPP__
#define SRC_COMPONENTS_CRYPT_ROOMLAVAPITCELLEFFECT_HPP__

#include <Components/Position.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace Game::Cmp::Crypt
{

//! @brief Marker component on a Cmp::Crypt::RoomLavaPitCell used to query all currently running lava
//! bubbling/animation effects.
class RoomLavaPitCellEffect
{
public:
  //! @brief Whether this effect is currently active.
  bool active{ true };
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_ROOMLAVAPITCELLEFFECT_HPP__
