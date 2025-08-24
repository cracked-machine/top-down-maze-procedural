#ifndef __COMPONENTS_OBSTACLE_HPP__
#define __COMPONENTS_OBSTACLE_HPP__

#include <Random.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Cmp {

// Obstacle concept
class Obstacle {
public:

    enum class Type
    {
        NONE = 0,
        BRICK,
        BEDROCK
    };

    Obstacle(
        unsigned int tile_pick,
        Type type = Type::BRICK, 
        bool visible = true, 
        bool enabled = true
    )
        :
        m_visible(visible),
        m_enabled(enabled),
        m_type(type),
        m_tile_pick(tile_pick)
    {
        
    }

    bool m_visible{true};
    bool m_enabled{true};
    bool m_broken{false};
    
    Type m_type;
    // uint8_t neighbours{0};
    // bool m_armed{false}; // has an active bomb
    unsigned int m_tile_pick{0};

private:
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_OBSTACLE_HPP__