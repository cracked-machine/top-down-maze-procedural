#ifndef __SYS_NPCSYSTEM_HPP__
#define __SYS_NPCSYSTEM_HPP__

#include <Armed.hpp>
#include <EnttDistanceSet.hpp>
#include <Loot.hpp>
#include <Movement.hpp>
#include <NPC.hpp>
#include <NPCDistance.hpp>
#include <Neighbours.hpp>
#include <Obstacle.hpp>
#include <PlayableCharacter.hpp>
#include <Position.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <Settings.hpp>
#include <SpriteFactory.hpp>
#include <entt/entity/entity.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys {

class NpcSystem {
public:
    NpcSystem(
        std::shared_ptr<entt::basic_registry<entt::entity>> reg
    )
    :
        m_reg(reg)
    {
    }

    void update_positions(sf::Time dt) {
        auto view = m_reg->view<Cmp::NPC, Cmp::Position, Cmp::Movement>();
        for (auto [entity, npc, pos, movement] : view.each()) {
            pos.x += movement.velocity.x * dt.asSeconds();
            pos.y += movement.velocity.y * dt.asSeconds();
        }
    }

    // find nearest obstacles and add them to EnttDistanceSet
    void update_paths(entt::entity npc_entity)
    {

        Cmp::EnttDistanceSet distance_set;
        for (auto [obstacle_entity, next_pos]: m_reg->view<Cmp::Position>(entt::exclude<Cmp::NPC, Cmp::PlayableCharacter>).each())
        {
            // skip any impassible obstacles
            auto possible_obstacle = m_reg->try_get<Cmp::Obstacle>(obstacle_entity);
            if(not possible_obstacle || possible_obstacle->m_enabled) continue;

            // Use manhattan distance to skip diagonal distances. 
            // This prevents NPC from going between diagonal obstacle gaps.
            int distance = getChebyshevDistance(getGridPosition(npc_entity), getGridPosition(obstacle_entity));
            if (distance < 2)
            {
                m_reg->emplace_or_replace<Cmp::NPCDistance>(obstacle_entity, 1);
                distance_set.set(obstacle_entity);
            }
        }

        m_reg->emplace_or_replace<Cmp::EnttDistanceSet>(npc_entity, distance_set);
    }

    // TODO Move these into BaseSystems.hpp
    sf::Vector2i getGridPosition(entt::entity entity) const
    {
        auto pos = m_reg->try_get<Cmp::Position>(entity);
        if (pos) {
            return { static_cast<int>(pos->x / Settings::OBSTACLE_SIZE_2F.x), static_cast<int>(pos->y / Settings::OBSTACLE_SIZE_2F.y) };
        }
        return { -1, -1 }; // Invalid position
    }
    // TODO Move these into BaseSystems.hpp
    unsigned int getManhattanDistance(sf::Vector2i posA, sf::Vector2i posB ) const
    {
        return std::abs(posA.x - posB.x) + std::abs(posA.y - posB.y);
    }

    // max( (posA.x - posB.x), (posA.y - posB.y) )
    // cardinal and diagonal directions
    // TODO Move these into BaseSystems.hpp
    unsigned int getChebyshevDistance(sf::Vector2i posA, sf::Vector2i posB ) const
    {
        return std::max(std::abs(posA.x - posB.x), std::abs(posA.y - posB.y));
    }
private:
    std::shared_ptr<entt::basic_registry<entt::entity>> m_reg;
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_NPCSYSTEM_HPP__
