#ifndef __SYS_LERPSYSTEM_HPP__
#define __SYS_LERPSYSTEM_HPP__

#include <NPCScanBounds.hpp>
#include <Settings.hpp>
#include <Systems/BaseSystem.hpp>
#include <Components/Position.hpp>
#include <Components/LerpPosition.hpp>
#include <algorithm>
#include <cmath>

namespace ProceduralMaze::Sys {

class LerpSystem : public BaseSystem {
public:
    LerpSystem(std::shared_ptr<entt::basic_registry<entt::entity>> reg) 
        : BaseSystem(reg) {}

    void update(sf::Time dt) {
        auto view = m_reg->view<Cmp::Position, Cmp::LerpPosition, Cmp::NPCScanBounds>();

        for (auto [entity, pos, target, npc_scan_bounds] : view.each()) {
            // If this is the first update, store the start position
            if (target.m_lerp_factor == 0.0f) {
                target.m_start = pos;
            }

            target.m_lerp_factor += target.m_lerp_speed * dt.asSeconds();
            
            if (target.m_lerp_factor >= 1.0f) {
                pos = target.m_target;
                m_reg->remove<Cmp::LerpPosition>(entity);
            } else {
                // Lerp from start to target directly
                pos.x = std::lerp(target.m_start.x, target.m_target.x, target.m_lerp_factor);
                pos.y = std::lerp(target.m_start.y, target.m_target.y, target.m_lerp_factor);
            }
            
            m_reg->patch<Cmp::NPCScanBounds>(entity, [&](auto &npc_scan_bounds){ 
                npc_scan_bounds.position(pos); 
            });
        }
    }

private:
    
    // sf::Vector2f lerp(const sf::Vector2f& start, const sf::Vector2f& end, float factor) {
    //     return start + (end - start) * factor;
    //     // Result = (1 - factor) * start + factor * end
    //     // Result = start + factor * (end - start)
    // }
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_LERPSYSTEM_HPP__