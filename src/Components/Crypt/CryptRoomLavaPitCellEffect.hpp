#ifndef SRC_COMPONENTS_CRYPTROOMLAVAPITCELLEFFECT_HPP
#define SRC_COMPONENTS_CRYPTROOMLAVAPITCELLEFFECT_HPP

#include <Components/Position.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace Game::Cmp
{

// this is just a marker so we can easily query all existing animations
class CryptRoomLavaPitCellEffect
{
public:
  bool active{ true };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_CRYPTROOMLAVAPITCELLEFFECT_HPP