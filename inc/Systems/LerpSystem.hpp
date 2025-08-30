#ifndef __SYS_LERPSYSTEM_HPP__
#define __SYS_LERPSYSTEM_HPP__

#include <Systems/BaseSystem.hpp>
#include <Components/Position.hpp>
#include <Components/LerpPosition.hpp>

namespace ProceduralMaze::Sys {

class LerpSystem : public BaseSystem {
public:
    LerpSystem(std::shared_ptr<entt::basic_registry<entt::entity>> reg) 
        : BaseSystem(reg) {}

    void update(sf::Time dt) {
        auto view = m_reg->view<Cmp::Position, Cmp::LerpPosition>();
        
        for (auto [entity, pos, target] : view.each()) {
            // Update lerp factor
            target.m_lerp_factor += target.m_move_speed * target.m_time_scale * dt.asSeconds();
            
            if (target.m_lerp_factor >= 1.0f) {
                // Movement complete - snap to final position
                pos = target.m_target;
                m_reg->remove<Cmp::LerpPosition>(entity);
            } else {
                // Interpolate position
                pos = lerp(pos, target.m_target, target.m_lerp_factor);
            }
        }
    }

private:
    sf::Vector2f lerp(const sf::Vector2f& start, const sf::Vector2f& end, float factor) {
        return start + (end - start) * factor;
        // Result = (1 - factor) * start + factor * end
        // Result = start + factor * (end - start)
    }
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_LERPSYSTEM_HPP__