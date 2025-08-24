#ifndef __COMPONENTS_BOMB_SYSTEM_HPP__
#define __COMPONENTS_BOMB_SYSTEM_HPP__

#include <Armed.hpp>
#include <Neighbours.hpp>
#include <Obstacle.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys {

// this currently only supports one bomb at a time
class BombSystem {
public:
    BombSystem(std::shared_ptr<entt::basic_registry<entt::entity>> reg) : m_reg(reg) { }

    void update()
    {
        auto armed_view = m_reg->view<Cmp::Armed, Cmp::Obstacle, Cmp::Neighbours>();
        for( auto [_entt, _armed_cmp, _obstacle_cmp, _neighbours_cmp]: armed_view.each() ) 
        {
            if (_armed_cmp.getElapsedTime() < detonation_delay) continue;

            for( auto [dir, neighbour_entity] : _neighbours_cmp) 
            {
                if( not m_reg->valid(entt::entity(neighbour_entity)) ) 
                {
                    SPDLOG_WARN("List provided invalid neighbour entity: {}", entt::to_integral(neighbour_entity));
                    assert(m_reg->valid(entt::entity(neighbour_entity)) && "List provided invalid neighbour entity: " 
                        && entt::to_integral(neighbour_entity));
                    continue;
                }

                Cmp::Obstacle* nb_obstacle = m_reg->try_get<Cmp::Obstacle>(entt::entity(neighbour_entity));
                if( not nb_obstacle )
                {
                    SPDLOG_WARN("Unable to find Obstacle component for entity: {}", entt::to_integral(neighbour_entity));
                    assert(nb_obstacle && "Unable to find Obstacle component for entity: "  && entt::to_integral(neighbour_entity));
                    continue;
                }
                if (nb_obstacle->m_enabled && not nb_obstacle->m_broken)
                {
                    // tell the render system to draw detonated obstacle differently
                    nb_obstacle->m_broken = true;
                    nb_obstacle->m_enabled = false;
                }
            }

            // if we got this far then the bomb detonated, we can destroy the armed component
            m_reg->erase<Cmp::Armed>(_entt);
        }
    }

private:
    std::shared_ptr<entt::basic_registry<entt::entity>> m_reg;
    sf::Time detonation_delay{sf::seconds(3)};
};

} // namespace ProceduralMaze::Sys

#endif // __COMPONENTS_BOMB_SYSTEM_HPP__