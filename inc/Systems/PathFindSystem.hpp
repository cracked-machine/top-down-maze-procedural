#ifndef __SYS_PATHFINDSYSTEM_HPP__
#define __SYS_PATHFINDSYSTEM_HPP__

#include <DijkstraDistance.hpp>
#include <Obstacle.hpp>
#include <Position.hpp>
#include <PlayableCharacter.hpp>
#include <VisitedNode.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>
#include <queue>
#include <vector>
#include <utility> // for std::pair
#include <functional> // for std::greater
#include <cmath> // for std::sqrt
#include <limits> // for std::numeric_limits
#include <array>
#include <unordered_map>
#include <spdlog/spdlog.h>
#include <Settings.hpp>

namespace ProceduralMaze::Sys {

class PathFindSystem {
public:
    PathFindSystem(std::shared_ptr<entt::basic_registry<entt::entity>> reg) : m_reg(std::move(reg)) {}
    ~PathFindSystem() = default;

    void findPath(entt::entity start_entity, entt::entity end_entity)
    {
        m_start_entity = start_entity;
        m_end_entity = end_entity;


        // SPDLOG_INFO("Start entity {}, End entity {}", static_cast<uint32_t>(m_start_entity), static_cast<uint32_t>(m_end_entity));
        for( auto [entity, _pos]: m_reg->view<Cmp::Position>().each() )
        {
            if (entity == start_entity) {
                m_reg->emplace_or_replace<Cmp::DijkstraDistance>(entity, 0);
                
            } else {
                m_reg->emplace_or_replace<Cmp::DijkstraDistance>(entity, std::numeric_limits<unsigned int>::max());
            }
        }
        if(getManhattenDistance(getGridPosition(m_start_entity), getGridPosition(m_end_entity)) > AGGRO_DISTANCE) 
        {
            // m_reg->clear<Cmp::DijkstraDistance>();   
            return;
        }
        m_priority_queue.push({0, m_start_entity});
        check_unvisited_set();
    }

    void check_unvisited_set()
    {
        std::pair<int, entt::entity> this_node = m_priority_queue.top();

        for (auto [next_entity, next_pos]: m_reg->view<Cmp::Position>(entt::exclude<Cmp::VisitedNode>).each())
        {
            // disregard any obstacles that are collidable
            auto possible_obstacle = m_reg->try_get<Cmp::Obstacle>(next_entity);
            if(possible_obstacle && possible_obstacle->m_enabled) continue;

            int distance = getManhattenDistance(getGridPosition(this_node.second),  getGridPosition(next_entity));
            if (distance > SCAN_DISTANCE) continue;
            else m_reg->emplace_or_replace<Cmp::DijkstraDistance>(
                next_entity, 
                distance
            );

        }
    
    }

    std::vector<entt::entity> getPath()
    {
        // Reconstruct the path by following decreasing distances from end to start
        std::vector<entt::entity> path;

        return path;
    }

    sf::Vector2i getGridPosition(entt::entity entity) const
    {
        auto pos = m_reg->try_get<Cmp::Position>(entity);
        if (pos) {
            return { static_cast<int>(pos->x / Settings::OBSTACLE_SIZE_2F.x), static_cast<int>(pos->y / Settings::OBSTACLE_SIZE_2F.y) };
        }
        return { -1, -1 }; // Invalid position
    }

    int getManhattenDistance(sf::Vector2i posA, sf::Vector2i posB ) const
    {
        return std::abs(posA.x - posB.x) + std::abs(posA.y - posB.y);
    }

private:
    std::shared_ptr<entt::basic_registry<entt::entity>> m_reg;

    // Define possible movement directions (up, right, down, left)
    const std::array<sf::Vector2f, 4> m_directions = {
        sf::Vector2f(0.f, -Settings::OBSTACLE_SIZE.y),  // Up
        sf::Vector2f(Settings::OBSTACLE_SIZE.x, 0.f),   // Right
        sf::Vector2f(0.f, Settings::OBSTACLE_SIZE.y),   // Down
        sf::Vector2f(-Settings::OBSTACLE_SIZE.x, 0.f)   // Left
    };

    entt::entity m_start_entity, m_end_entity;

    std::priority_queue<std::pair<int, entt::entity>, 
        std::vector<std::pair<int, entt::entity>>, 
        std::greater<std::pair<int, entt::entity>>> m_priority_queue;

    const int SCAN_DISTANCE{5};
    const int AGGRO_DISTANCE{5};
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_PATHFINDSYSTEM_HPP__