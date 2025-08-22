#ifndef __COMPONENTS_WATER_LEVEL_HPP__
#define __COMPONENTS_WATER_LEVEL_HPP__

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Cmp {

class WaterLevel {
public:
    WaterLevel(float initial_level, float flood_velocity) 
    : level(initial_level), m_flood_velocity(flood_velocity) 
    {}

    float get_level() const { return level; }
    // void set_level(float new_level) { level = new_level; }
    void update()
    {
        if( m_clock.getElapsedTime() > sf::seconds(dt) )
        {
            level -= (dt * m_flood_velocity);
            m_clock.restart();
        }
    }
private:
    float level;
    float dt = 1.0f;
    [[maybe_unused]] float m_flood_velocity; // rate at which the water level rises

    sf::Clock m_clock;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_WATER_LEVEL_HPP__