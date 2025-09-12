#ifndef __COMPONENT_MOVEMENT_HPP__
#define __COMPONENT_MOVEMENT_HPP__

#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze {
namespace Cmp {

struct Movement
{
  // Maximum speed in pixels per second
  float max_speed{};
  // Base friction coefficient when colliding
  float friction_coefficient{};
  // How quickly friction decreases with speed (0-1)
  float friction_falloff{};
  // Acceleration in pixels per second squared
  float above_water_default_acceleration_rate{};
  // Deceleration when no input in pixels per second squared
  float above_water_default_deceleration_rate{};
  // Acceleration in pixels per second squared
  float under_water_default_acceleration_rate{};
  // Deceleration when no input in pixels per second squared
  float under_water_default_deceleration_rate{};

  // Change in velocity = acceleration * dt.
  // Change in position = velocity * dt.
  sf::Vector2f velocity{ 0.0f, 0.0f };
  sf::Vector2f acceleration{ 0.0f, 0.0f };
  float acceleration_rate = default_acceleration_rate;
  float deceleration_rate = default_deceleration_rate;
  float min_velocity = 1.f;                 // Minimum velocity before stopping completely
  bool is_colliding = false;                // Flag to track if entity is currently colliding
  float default_acceleration_rate = 500.0f; // Default acceleration rate
  float default_deceleration_rate = 600.0f; // Default deceleration rate
  const float DEFAULT_MAX_SPEED = 100.0f;   // Default maximum speed
};

} // namespace Cmp
} // namespace ProceduralMaze

#endif // __COMPONENT_MOVEMENT_HPP__
