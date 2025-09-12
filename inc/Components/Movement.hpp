#ifndef __COMPONENT_MOVEMENT_HPP__
#define __COMPONENT_MOVEMENT_HPP__

#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze {
namespace Cmp {

struct Movement
{

  // Change in velocity = acceleration * dt.
  // Change in position = velocity * dt.
  float acceleration_rate{};
  float deceleration_rate{};
  sf::Vector2f velocity{ 0.0f, 0.0f };
  sf::Vector2f acceleration{ 0.0f, 0.0f };
  float min_velocity = 1.f;  // Minimum velocity before stopping completely
  bool is_colliding = false; // Flag to track if entity is currently colliding

  const float DEFAULT_MAX_SPEED = 100.0f; // Default maximum speed
};

} // namespace Cmp
} // namespace ProceduralMaze

#endif // __COMPONENT_MOVEMENT_HPP__
