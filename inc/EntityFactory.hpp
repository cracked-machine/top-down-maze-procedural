#ifndef __ENTITY_FACTORY_HPP__
#define __ENTITY_FACTORY_HPP__

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

#include <Sprites/Brick.hpp>
#include <Sprites/Player.hpp>

namespace ProceduralMaze {

class EntityFactory
{
public:
    EntityFactory(EntityFactory const&) = delete;
    EntityFactory operator=(EntityFactory const&) = delete;

    static void add_system_entity(std::shared_ptr<entt::basic_registry<entt::entity>> reg)
    {
        SPDLOG_INFO("Creating system entity");
        auto entity = reg->create();
        reg->emplace<Cmp::System>(entity);
    }


    static void add_player_entity(std::shared_ptr<entt::basic_registry<entt::entity>> reg)
    {
        SPDLOG_INFO("Creating player entity");
        auto entity = reg->create();
        reg->emplace<Cmp::Position>(entity, Settings::PLAYER_START_POS);
        reg->emplace<Cmp::PlayableCharacter>(entity);
        reg->emplace<Cmp::Movement>(entity);
    }


private:
    EntityFactory() {}
};

} // namespace ProceduralMaze

#endif // __ENTITY_FACTORY_HPP__