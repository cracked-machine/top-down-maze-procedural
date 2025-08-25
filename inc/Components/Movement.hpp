#ifndef __COMPONENT_MOVEMENT_HPP__
#define __COMPONENT_MOVEMENT_HPP__

#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze {
namespace Cmp {

struct Movement {
    sf::Vector2f velocity{0.0f, 0.0f};
    sf::Vector2f acceleration{0.0f, 0.0f};
    float max_speed = 100.0f;           // Maximum speed in pixels per second
    float acceleration_rate = 500.0f;    // Acceleration in pixels per second squared
    float deceleration_rate = 600.0f;    // Deceleration when no input in pixels per second squared
    float friction_coefficient = 0.02f;   // Base friction coefficient when colliding
    float friction_falloff = 0.5f;      // How quickly friction decreases with speed (0-1)
    float min_velocity = 1.f;          // Minimum velocity before stopping completely
    bool is_colliding = false;          // Flag to track if entity is currently colliding
};

} // namespace Cmp
} // namespace ProceduralMaze

#endif // __COMPONENT_MOVEMENT_HPP__
