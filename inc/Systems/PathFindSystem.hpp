#ifndef __SYS_PATHFINDSYSTEM_HPP__
#define __SYS_PATHFINDSYSTEM_HPP__

#include <Systems/BaseSystem.hpp>
#include <Components/NPCDistance.hpp>
#include <Components/PlayerDistance.hpp>
#include <Components/EnttDistanceSet.hpp>
#include <Components/NPC.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Position.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Settings.hpp>

#include <LerpPosition.hpp>
#include <cstdlib>

#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>

#include <cmath> // for std::sqrt
#include <limits> // for std::numeric_limits
#include <array>

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
        resetNonNpcDistances();
        for (auto [npc_entity, npc_cmp, player_distance_from_npc]: m_reg->view<Cmp::NPC, Cmp::PlayerDistance>().each())
        {
            updatePlayerDistanceFrom(npc_entity, 5);
            updateDistancesFrom(npc_entity, player_entity);
        }
    }

    void updateDistancesFrom(entt::entity npc_entity, entt::entity player_entity)
    {
        // only continue if we are within aggro distance
        auto npc_entity_grid_pos = getGridPosition(npc_entity);
        auto player_entity_grid_pos = getGridPosition(player_entity);
        if (not npc_entity_grid_pos || not player_entity_grid_pos) return;
        auto start_end_entity_distance = getChebyshevDistance(npc_entity_grid_pos.value(), player_entity_grid_pos.value());
        
        if (start_end_entity_distance < AGGRO_DISTANCE) 
        {
            // first update the obstacles with their NPC distances
            Cmp::EnttDistanceSet distance_set;
            for (auto [obstacle_entity, next_pos]: m_reg->view<Cmp::Position>(entt::exclude<Cmp::NPC, Cmp::PlayableCharacter>).each())
            {
                // skip any impassible obstacles
                auto possible_obstacle = m_reg->try_get<Cmp::Obstacle>(obstacle_entity);
                if(not possible_obstacle || possible_obstacle->m_enabled) continue;         

                // Use manhattan distance to skip diagonal distances. 
                // This prevents NPC from going between diagonal obstacle gaps.
                auto npc_entity_grid_pos = getGridPosition(npc_entity);
                auto obstacle_entity_grid_pos = getGridPosition(obstacle_entity);
                if( not npc_entity_grid_pos || not obstacle_entity_grid_pos) continue;

                // chebychev will allow NPC to scan for possible diagonal movement but we limit it so that it doesnt scan over walls
                int distance = getChebyshevDistance(npc_entity_grid_pos.value(), obstacle_entity_grid_pos.value());
                if (distance <= SCAN_DISTANCE)
                {
                    m_reg->emplace_or_replace<Cmp::NPCDistance>(obstacle_entity, distance);
                    distance_set.set(obstacle_entity);
                }
            }

            // Add the obstacle distance set to the NPC - this is mostly so we can display it on the screen later
            m_reg->emplace_or_replace<Cmp::EnttDistanceSet>(npc_entity, distance_set);

            // Get the known player distance (Cmp::PlayerDistance) stored in the NPCs entity
            auto player_distance_cmp = m_reg->try_get<Cmp::PlayerDistance>(npc_entity);
            if( not player_distance_cmp) return;

            // now for each candidate in NPCs Cmp::EnttDistanceSet, check if one moves us closer to the player
            for(auto move_candidate: distance_set)
            {
                // Get grid positions for comparison
                auto candidate_grid_pos = getGridPosition(move_candidate);
                auto player_grid_pos = getGridPosition(player_entity);
                auto npc_grid_pos = getGridPosition(npc_entity);
                if( not candidate_grid_pos || not player_grid_pos || not npc_grid_pos) continue;

                // Use Manhattan distance when actually moving to prevent jumping over walls
                auto current_distance = getManhattanDistance(npc_grid_pos.value(), player_grid_pos.value());
                auto new_distance = getManhattanDistance(candidate_grid_pos.value(), player_grid_pos.value());

                // Move if this position gets us closer to the player
                if (new_distance < current_distance)
                {
                    auto npc_cmp = m_reg->try_get<Cmp::NPC>(npc_entity);
                    if(npc_cmp) {
                        // prevent the NPC from moving too fast
                        if(npc_cmp->m_move_cooldown.getElapsedTime() < npc_cmp->MOVE_DELAY) continue;
                        auto move_candidate_pixel_pos = getPixelPosition(move_candidate);
                        if (not move_candidate_pixel_pos) continue;

                        // Set target position instead of directly moving
                        // Start lerp factor at 0   
                        m_reg->emplace_or_replace<Cmp::LerpPosition>(npc_entity, move_candidate_pixel_pos.value(), 0.0f);

                        npc_cmp->m_move_cooldown.restart();

                        // Update the stored distance
                        m_reg->emplace_or_replace<Cmp::PlayerDistance>(npc_entity, new_distance);
                        break; // Take the first better move we find
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
            auto npc_entity_grid_pos = getGridPosition(npc_entity);
            auto player_entity_grid_pos = getGridPosition(player_entity);
            if( not npc_entity_grid_pos || not player_entity_grid_pos) continue;

            int distance = getChebyshevDistance(npc_entity_grid_pos.value(), player_entity_grid_pos.value());
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

private:


    // Define possible movement directions (up, right, down, left)
    const std::array<sf::Vector2f, 4> m_directions = {
        sf::Vector2f(0.f, -Settings::OBSTACLE_SIZE.y),  // Up
        sf::Vector2f(Settings::OBSTACLE_SIZE.x, 0.f),   // Right
        sf::Vector2f(0.f, Settings::OBSTACLE_SIZE.y),   // Down
        sf::Vector2f(-Settings::OBSTACLE_SIZE.x, 0.f)   // Left
    };

    // entt::entity m_start_entity, m_end_entity;



    const int SCAN_DISTANCE{1};
    const unsigned int AGGRO_DISTANCE{10};
    const unsigned int MAX_DISTANCE{std::numeric_limits<unsigned int>::max()};
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_PATHFINDSYSTEM_HPP__