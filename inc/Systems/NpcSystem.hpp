#ifndef __SYS_NPCSYSTEM_HPP__
#define __SYS_NPCSYSTEM_HPP__

#include <Direction.hpp>
#include <LerpPosition.hpp>
#include <Movement.hpp>
#include <NPC.hpp>
#include <NPCScanBounds.hpp>
#include <Position.hpp>
#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys {

class NpcSystem {
public:
    NpcSystem(std::shared_ptr<entt::basic_registry<entt::entity>> reg)
        : m_reg(reg) {}

    void add_npc_entity(sf::Vector2f position)
    {
        auto new_npc_entity = m_reg->create();
        m_reg->emplace<Cmp::NPC>(new_npc_entity, true);
        m_reg->emplace<Cmp::Position>(new_npc_entity, position);
        m_reg->emplace<Cmp::NPCScanBounds>(new_npc_entity, position, Settings::OBSTACLE_SIZE_2F);
    }

    void remove_npc_entity(entt::entity npc_entity)
    {
        // kill npc
        m_reg->remove<Cmp::NPC>(npc_entity);
        m_reg->remove<Cmp::Position>(npc_entity);
        m_reg->remove<Cmp::NPCScanBounds>(npc_entity);
    }

    void lerp_movement(sf::Time dt) {
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
    std::shared_ptr<entt::basic_registry<entt::entity>> m_reg;
};

} // namespace Sys

#endif // __SYS_NPCSYSTEM_HPP__
