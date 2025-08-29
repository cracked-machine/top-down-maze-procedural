#ifndef __SYS_PATHFINDSYSTEM_HPP__
#define __SYS_PATHFINDSYSTEM_HPP__

#include <NPCDistance.hpp>
#include <PlayerDistance.hpp>
#include <EnttDistanceSet.hpp>
#include <NPC.hpp>
#include <Obstacle.hpp>
#include <Position.hpp>
#include <PlayableCharacter.hpp>
#include <cstdlib>
#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>
#include <utility> // for std::pair
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

    void findPath(entt::entity player_entity)
    {

        resetNonNpcDistances();
        for (auto [npc_entity, npc_cmp, npc_dijkstra_cmp]: m_reg->view<Cmp::NPC, Cmp::PlayerDistance>().each())
        {
            updatePlayerDistanceFrom(npc_entity, 5);
            getDistancesFrom(npc_entity, player_entity);
        }
    }

    void getDistancesFrom(entt::entity npc_entity, entt::entity player_entity)
    {

        // only continue if we are within aggro distance
        auto start_end_entity_distance = getManhattenDistance(getGridPosition(npc_entity), getGridPosition(player_entity));
        if (start_end_entity_distance < AGGRO_DISTANCE) 
        {
            Cmp::EnttDistanceSet distance_set;
            for (auto [next_entity, next_pos]: m_reg->view<Cmp::Position>(entt::exclude<Cmp::NPC, Cmp::PlayableCharacter>).each())
            {
                // skip any impassible tiles
                auto possible_obstacle = m_reg->try_get<Cmp::Obstacle>(next_entity);
                if(possible_obstacle && possible_obstacle->m_enabled) continue;

                // record entity for disabled obstacles that are only 1 block distance
                int distance = getManhattenDistance(getGridPosition(npc_entity), getGridPosition(next_entity));
                if (distance == 1)
                {
                    m_reg->emplace_or_replace<Cmp::NPCDistance>(next_entity, 1);
                    distance_set.set(next_entity);
                }
            }
            m_reg->emplace_or_replace<Cmp::EnttDistanceSet>(npc_entity, distance_set);

            auto player_distance_cmp = m_reg->try_get<Cmp::PlayerDistance>(npc_entity);
            if( not player_distance_cmp) return;
            for(auto move_candidate: distance_set)
            {
                if( getManhattenDistance(getGridPosition(npc_entity), getGridPosition(move_candidate)) < player_distance_cmp->distance)
                {
                    auto npc_cmp = m_reg->try_get<Cmp::NPC>(npc_entity);
                    if(npc_cmp) {
                        if(npc_cmp->m_move_cooldown.getElapsedTime() < npc_cmp->MOVE_DELAY) continue;
                        m_reg->emplace_or_replace<Cmp::Position>(npc_entity, getPixelPosition(move_candidate));
                        npc_cmp->m_move_cooldown.restart();
                    }
                }
            }
        }
        else
        {
            // now NPC is out of aggro range, remove their pathing data
            m_reg->remove<Cmp::EnttDistanceSet>(npc_entity);
        }
        
        
    }

    void updatePlayerDistanceFrom(entt::entity npc_entity, int aggro_distance)
    {
        auto player_only_view = m_reg->view<Cmp::Position, Cmp::PlayableCharacter>();
        for (auto [player_entity, player_pos ,player]: player_only_view.each())
        {
            int distance = getManhattenDistance(getGridPosition(npc_entity), getGridPosition(player_entity));
            if(distance < aggro_distance) {
                m_reg->emplace_or_replace<Cmp::PlayerDistance>(npc_entity, distance);
            }
        }
    }

    // reset distances of all non-NPC entities
    void resetNonNpcDistances()
    {
        for( auto [entity, _pos]: m_reg->view<Cmp::Position>().each() ) 
        {
            m_reg->emplace_or_replace<Cmp::NPCDistance>(entity, MAX_DISTANCE);
        }
    }

    sf::Vector2i getGridPosition(entt::entity entity) const
    {
        auto pos = m_reg->try_get<Cmp::Position>(entity);
        if (pos) {
            return { static_cast<int>(pos->x / Settings::OBSTACLE_SIZE_2F.x), static_cast<int>(pos->y / Settings::OBSTACLE_SIZE_2F.y) };
        }
        return { -1, -1 }; // Invalid position
    }

    sf::Vector2f getPixelPosition(entt::entity entity) const
    {
        auto pos = m_reg->try_get<Cmp::Position>(entity);
        if (pos) { return *pos; }
        return { -1, -1 }; // Invalid position
    }

    unsigned int getManhattenDistance(sf::Vector2i posA, sf::Vector2i posB ) const
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
    const unsigned int AGGRO_DISTANCE{5};
    const unsigned int MAX_DISTANCE{std::numeric_limits<unsigned int>::max()};
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_PATHFINDSYSTEM_HPP__