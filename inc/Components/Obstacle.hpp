#ifndef __COMPONENTS_OBSTACLE_HPP__
#define __COMPONENTS_OBSTACLE_HPP__

#include <Random.hpp>
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
        const std::vector<unsigned int> &tile_picks, 
        Type type = Type::BRICK, 
        bool visible = true, 
        bool enabled = true
    )
        :
        m_random_tile_picker(0, tile_picks.size() - 1),
        m_visible(visible),
        m_enabled(enabled),
        m_type(type)
    {
        // pick a random tile from the provided possible choices `tile_picks`
        m_tile_pick = tile_picks[m_random_tile_picker.gen()];
    }

    bool m_visible{true};
    bool m_enabled{true};
    Type m_type;
    uint8_t neighbours{0};
    unsigned int m_tile_pick{0};

private:
    Cmp::Random m_random_tile_picker;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_OBSTACLE_HPP__