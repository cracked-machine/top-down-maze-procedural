#ifndef __SYS_PATHFINDSYSTEM_HPP__
#define __SYS_PATHFINDSYSTEM_HPP__

#include <DijkstraDistance.hpp>
#include <NPC.hpp>
#include <Obstacle.hpp>
#include <Position.hpp>
#include <PlayableCharacter.hpp>
#include <VisitedNode.hpp>
#include <cstdlib>
#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>
#include <queue>
#include <sstream>
#include <string>
#include <vector>
#include <utility> // for std::pair
#include <functional> // for std::greater
#include <cmath> // for std::sqrt
#include <limits> // for std::numeric_limits
#include <array>
#include <spdlog/spdlog.h>
#include <Settings.hpp>
#include <Components/EnttDistancePriorityQueue.hpp>

namespace ProceduralMaze::Sys {


class PathFindSystem {
public:
    PathFindSystem(std::shared_ptr<entt::basic_registry<entt::entity>> reg) : m_reg(std::move(reg)) {}
    ~PathFindSystem() = default;

    void findPath(entt::entity end_entity)
    {

        resetNonNpcDistances();
        for( auto [e,distance_queue] : m_reg->view<Cmp::EnttDistancePriorityQueue>().each()) distance_queue.clear();
        for (auto [npc_entity, npc_cmp, npc_dijkstra_cmp]: m_reg->view<Cmp::NPC, Cmp::DijkstraDistance>().each())
        {
            getDistancesFrom(npc_entity, end_entity, 2);
            updatePlayerDistanceFrom(npc_entity, 5);
        }
    }

    void getDistancesFrom(entt::entity from_entity, entt::entity to_entity, int scan_distance = 2)
    {

        auto dijkstra_cmp = m_reg->try_get<Cmp::DijkstraDistance>(from_entity);
        if( not dijkstra_cmp) return;

        auto entt_distance_priority_queue_view = m_reg->view<Cmp::EnttDistancePriorityQueue>();
        for( auto [e,distance_queue] : entt_distance_priority_queue_view.each())
        {   
            // only continue if we are within aggro distance
            auto start_end_entity_distance = getManhattenDistance(getGridPosition(from_entity), getGridPosition(to_entity));
            if (start_end_entity_distance < AGGRO_DISTANCE) 
            {
                dijkstra_cmp->distance = 0; 
    
                // Update distances for nearby (disabled) obstacle entities
                std::vector<Cmp::EnttDistances> nearby_entities;
                for (auto [next_entity, next_pos]: m_reg->view<Cmp::Position>(entt::exclude<Cmp::NPC, Cmp::PlayableCharacter>).each())
                {
                    // skip any impassible tiles
                    auto possible_obstacle = m_reg->try_get<Cmp::Obstacle>(next_entity);
                    if(possible_obstacle && possible_obstacle->m_enabled) continue;
    
                    // otherwise update distance
                    int distance = getManhattenDistance(getGridPosition(from_entity), getGridPosition(next_entity));
                    if (distance <= scan_distance)
                    {
                        nearby_entities.emplace_back(distance,next_entity);
                    }
                }

                if (std::any_of(nearby_entities.begin(), nearby_entities.end(),
                    [](const auto& pair) { return pair.first == 1; }))
                {
                    for(const auto& nearby_entity: nearby_entities)
                    {
                        m_reg->emplace_or_replace<Cmp::DijkstraDistance>(nearby_entity.second, nearby_entity.first);
                        distance_queue.push(nearby_entity);
                    }
                }

            }

        }


        // now we are done....mark this node as visited
        m_reg->emplace_or_replace<Cmp::VisitedNode>(from_entity, true);          
        
    }

    void updatePlayerDistanceFrom(entt::entity from_entity, int aggro_distance)
    {
        auto player_only_view = m_reg->view<Cmp::Position, Cmp::PlayableCharacter>();
        for (auto [player_entity, player_pos ,player]: player_only_view.each())
        {
            int distance = getManhattenDistance(getGridPosition(from_entity), getGridPosition(player_entity));
            if(distance < aggro_distance) {
                m_reg->emplace_or_replace<Cmp::DijkstraDistance>(player_entity, distance);
            }
        }
    }

    // reset distances of all non-NPC entities
    void resetNonNpcDistances()
    {
        for( auto [entity, _pos]: m_reg->view<Cmp::Position>().each() ) 
        {
            m_reg->emplace_or_replace<Cmp::DijkstraDistance>(entity, DIJKSTRA_MAX_DISTANCE);
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

    // entt::entity m_start_entity, m_end_entity;



    // const int SCAN_DISTANCE{5};
    const int AGGRO_DISTANCE{5};
    const unsigned int DIJKSTRA_MAX_DISTANCE{std::numeric_limits<unsigned int>::max()};
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_PATHFINDSYSTEM_HPP__