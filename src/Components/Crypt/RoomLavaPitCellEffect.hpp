#ifndef SRC_COMPONENTS_CRYPT_ROOMLAVAPITCELLEFFECT_HPP__
#define SRC_COMPONENTS_CRYPT_ROOMLAVAPITCELLEFFECT_HPP__

#include <Components/Position.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace Game::Cmp::Crypt
{

// this is just a marker so we can easily query all existing animations
class RoomLavaPitCellEffect
{
public:
  bool active{ true };
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_ROOMLAVAPITCELLEFFECT_HPP__
