#ifndef __ENTITY_FACTORY_HPP__
#define __ENTITY_FACTORY_HPP__

#include <Direction.hpp>
#include <EnttDistancePriorityQueue.hpp>
#include <NPC.hpp>
#include <SFML/System/Vector2.hpp>
#include <WaterLevel.hpp>
#include <entt/entity/registry.hpp>

#include <Settings.hpp>

#include <Components/Font.hpp>
#include <Components/RandomCoord.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Position.hpp>
#include <Components/System.hpp>
#include <Components/Movement.hpp>
#include <Components/GameState.hpp>

namespace ProceduralMaze {

class EntityFactory
{
public:
    EntityFactory(EntityFactory const&) = delete;
    EntityFactory operator=(EntityFactory const&) = delete;

    static void add_system_entity(std::shared_ptr<entt::basic_registry<entt::entity>> reg)
    {
        if( not reg->view<Cmp::System>()->empty()) {
            SPDLOG_WARN("System entity already exists, skipping creation");
            return;
        }
        SPDLOG_INFO("Creating system entity");
        auto entity = reg->create();
        reg->emplace<Cmp::System>(entity);
    }


    static void add_player_entity(std::shared_ptr<entt::basic_registry<entt::entity>> reg)
    {
        if( not reg->view<Cmp::PlayableCharacter>()->empty()) {
            SPDLOG_WARN("Player entity already exists, skipping creation");
            return;
        }
        SPDLOG_INFO("Creating player entity");
        auto entity = reg->create();
        reg->emplace<Cmp::Position>(entity, Settings::PLAYER_START_POS);
        reg->emplace<Cmp::PlayableCharacter>(entity);
        reg->emplace<Cmp::Movement>(entity);
        reg->emplace<Cmp::Direction>(entity, sf::Vector2f{0,0});
    }


    static void add_npc_entity(std::shared_ptr<entt::basic_registry<entt::entity>> reg, sf::Vector2f position)
    {
        SPDLOG_INFO("Creating NPC entity");
        auto entity = reg->create();
        reg->emplace<Cmp::Position>(entity, position);
        reg->emplace<Cmp::NPC>(entity, true);
        reg->emplace<Cmp::Movement>(entity);
        reg->emplace<Cmp::Direction>(entity, sf::Vector2f{0,0});
    }

    static void add_game_state_entity(std::shared_ptr<entt::basic_registry<entt::entity>> reg)
    {
        if( not reg->view<Cmp::GameState>()->empty()) {
            SPDLOG_WARN("Game state entity already exists, skipping creation");
            return;
        }
        SPDLOG_INFO("Creating game state entity");
        auto entity = reg->create();
        reg->emplace<Cmp::GameState>(entity);
    }

    static void add_entt_distance_priority_queue(std::shared_ptr<entt::basic_registry<entt::entity>> reg)
    {
        if( not reg->view<Cmp::EnttDistancePriorityQueue>()->empty()) {
            SPDLOG_WARN("EnttDistancePriorityQueue entity already exists, skipping creation");
            return;
        }
        SPDLOG_INFO("Creating EnttDistancePriorityQueue entity");
        auto entity = reg->create();
        reg->emplace<Cmp::EnttDistancePriorityQueue>(entity);
    }

private:
    EntityFactory() {}
};

} // namespace ProceduralMaze

#endif // __ENTITY_FACTORY_HPP__