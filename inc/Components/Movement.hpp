#ifndef __COMPONENT_MOVEMENT_HPP__
#define __COMPONENT_MOVEMENT_HPP__

#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze {
namespace Cmp {

struct Movement {
    sf::Vector2f velocity{0.0f, 0.0f};
    sf::Vector2f acceleration{0.0f, 0.0f};
    float max_speed = 150.0f;           // Maximum speed in pixels per second
    float acceleration_rate = 500.0f;    // Acceleration in pixels per second squared
    float deceleration_rate = 600.0f;    // Deceleration when no input in pixels per second squared
};

} // namespace Cmp
} // namespace ProceduralMaze

#endif // __COMPONENT_MOVEMENT_HPP__
