#ifndef __COMPONENTS_OBSTACLE_HPP__
#define __COMPONENTS_OBSTACLE_HPP__

#include <Random.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <cstdint>
#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>
#include <Sprites/SpriteFactory.hpp>

namespace ProceduralMaze::Cmp {

// Obstacle concept
class Obstacle {
public:
    
    Obstacle(
        SpriteFactory::Type type, 
        uint32_t tile_index = 0,
        bool visible = true, 
        bool enabled = true
    )
        :
        m_type(type),
        m_tile_index(tile_index),
        m_visible(visible),
        m_enabled(enabled)
    {
        
    }

    SpriteFactory::Type m_type;
    uint32_t m_tile_index{0};

    bool m_visible{true};
    bool m_enabled{true};
    bool m_broken{false};


private:
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_OBSTACLE_HPP__