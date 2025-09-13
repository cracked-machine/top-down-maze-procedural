#ifndef __COMPONENT_MOVEMENT_HPP__
#define __COMPONENT_MOVEMENT_HPP__

#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze {
namespace Cmp {

struct Movement
{

  sf::Vector2f velocity{ 0.0f, 0.0f };
  sf::Vector2f acceleration{ 0.0f, 0.0f };
  bool is_colliding = false; // Flag to track if entity is currently colliding
};

} // namespace Cmp
} // namespace ProceduralMaze

#endif // __COMPONENT_MOVEMENT_HPP__
