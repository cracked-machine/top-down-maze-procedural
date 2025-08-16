#ifndef __COMPONENTS_OBSTACLE_HPP__
#define __COMPONENTS_OBSTACLE_HPP__

#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Cmp {

// Obstacle concept
class Obstacle {
public:

    enum class Type
    {
        NONE = 0,
        BRICK = 1
    };

    Obstacle(Type type = Type::BRICK, bool visible = true, bool enabled = true)
    :
        m_visible(visible),
        m_enabled(enabled),
        m_type(type)
    {}

    bool m_visible{true};
    bool m_enabled{true};
    Type m_type;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_OBSTACLE_HPP__