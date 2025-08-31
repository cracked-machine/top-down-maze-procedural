#ifndef __CMP_LERPPOSITION_HPP__
#define __CMP_LERPPOSITION_HPP__

#include <SFML/System/Vector2.hpp>
#include <entt/entity/fwd.hpp>

namespace ProceduralMaze::Cmp {

struct LerpPosition {
    sf::Vector2f m_target;
    // 0.0 = start position, 1.0 = target position
    float m_lerp_factor{0.0f};  
    // Units per second
    float m_move_speed{0.25f};   
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP_LERPPOSITION_HPP__