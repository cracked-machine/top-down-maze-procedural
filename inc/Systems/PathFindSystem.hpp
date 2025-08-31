#ifndef __SYS_PATHFINDSYSTEM_HPP__
#define __SYS_PATHFINDSYSTEM_HPP__

#include <NPCScanBounds.hpp>
#include <PCDetectionBounds.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <Systems/BaseSystem.hpp>
#include <Components/PlayerDistance.hpp>
#include <Components/EnttDistanceMap.hpp>
#include <Components/NPC.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Position.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/LerpPosition.hpp>
#include <Settings.hpp>

#include <cstdlib>

#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>

#include <cmath>
#include <array>

#include <queue>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys {

class PathFindSystem : public BaseSystem {
public:
    PathFindSystem(
        std::shared_ptr<entt::basic_registry<entt::entity>> reg
    ) : BaseSystem(reg) 
    {}
    ~PathFindSystem() = default;

    void findPath(entt::entity player_entity)
    {
        for (auto [npc_entity, npc_cmp]: m_reg->view<Cmp::NPC>().each())
        {
            updateDistancesFrom(npc_entity, player_entity);
        }
    }

    void updateDistancesFrom(entt::entity npc_entity, entt::entity player_entity)
    {
        auto npc_scan_bounds = m_reg->try_get<Cmp::NPCScanBounds>(npc_entity); 
        auto pc_detection_bounds = m_reg->try_get<Cmp::PCDetectionBounds>(player_entity);
        if( not npc_scan_bounds || not pc_detection_bounds) return;
        
        // only continue if we are within aggro distance
        if (not npc_scan_bounds->findIntersection(pc_detection_bounds->getBounds()))
        {
            // now NPC is out of aggro range, remove their pathing data
            m_reg->remove<Cmp::EnttDistanceMap>(npc_entity);
        }
        else
        {
            std::priority_queue<std::pair<int, entt::entity>, std::vector<std::pair<int, entt::entity>>, std::greater<std::pair<int, entt::entity>>> distance_queue;
            for (auto [obstacle_entity, next_pos, player_distance]: m_reg->view<Cmp::Position, Cmp::PlayerDistance>(entt::exclude<Cmp::NPC, Cmp::PlayableCharacter>).each())
            {
                if(npc_scan_bounds->findIntersection(sf::FloatRect(next_pos, Settings::OBSTACLE_SIZE_2F)))
                {
                    distance_queue.push({player_distance.distance, obstacle_entity});
                }
            }

            if(distance_queue.empty()) return;
            auto nearest_obstacle = distance_queue.top();

            
            auto npc_cmp = m_reg->try_get<Cmp::NPC>(npc_entity);
            if(npc_cmp) {

                auto npc_lerp_pos_cmp = m_reg->try_get<Cmp::LerpPosition>(npc_entity);
                if(npc_lerp_pos_cmp && npc_lerp_pos_cmp->m_lerp_factor < 1.0f) {
                    // do not interrupt mid lerp
                }
                else 
                {
                    // prevent the NPC from moving too fast
                    // if(npc_cmp->m_move_cooldown.getElapsedTime() < npc_cmp->MOVE_DELAY) return;
                    auto move_candidate_pixel_pos = getPixelPosition(nearest_obstacle.second);
                    if (not move_candidate_pixel_pos) return;

                    // Set target position instead of directly moving
                    // Start lerp factor at 0   
                    m_reg->emplace_or_replace<Cmp::LerpPosition>(npc_entity, move_candidate_pixel_pos.value(), 0.0f);
                    // m_reg->emplace_or_replace<Cmp::Position>(npc_entity, move_candidate_pixel_pos.value());
                    // npc_cmp->m_move_cooldown.restart();

                    // m_reg->patch<Cmp::NPCScanBounds>(npc_entity, [&](auto &npc_scan_bounds){ npc_scan_bounds.position(move_candidate_pixel_pos.value()); });
                }
            }            
        }
    }


private:

    // Define possible movement directions (up, right, down, left)
    const std::array<sf::Vector2f, 4> m_directions = {
        sf::Vector2f(0.f, -Settings::OBSTACLE_SIZE.y),  // Up
        sf::Vector2f(Settings::OBSTACLE_SIZE.x, 0.f),   // Right
        sf::Vector2f(0.f, Settings::OBSTACLE_SIZE.y),   // Down
        sf::Vector2f(-Settings::OBSTACLE_SIZE.x, 0.f)   // Left
    };

    // the limit for finding potential paths
    // const int SCAN_DISTANCE{1};
    // the activation distance for NPCs
    // const unsigned int AGGRO_DISTANCE{5};

};

} // namespace ProceduralMaze::Sys

#endif // __SYS_PATHFINDSYSTEM_HPP__