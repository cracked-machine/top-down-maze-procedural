#ifndef __ENTITY_FACTORY_HPP__
#define __ENTITY_FACTORY_HPP__

#include <entt/entity/registry.hpp>

#include <Settings.hpp>

#include <Components/Font.hpp>
#include <Components/RandomCoord.hpp>
#include <Components/XAxisHitBox.hpp>
#include <Components/YAxisHitBox.hpp>
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

    static void add_system_entity(entt::basic_registry<entt::entity> &m_reg)
    {
        auto entity = m_reg.create();
        m_reg.emplace<Cmp::System>(entity); 
    }

    
    static void add_player_entity(entt::basic_registry<entt::entity> &m_reg)
    {
        auto entity = m_reg.create();
        m_reg.emplace<Cmp::Position>(entity, Settings::PLAYER_START_POS); 
        m_reg.emplace<Cmp::PlayableCharacter>(entity);
        m_reg.emplace<Cmp::Movement>(entity);
    }

private:
    EntityFactory() {}
};

} // namespace ProceduralMaze

#endif // __ENTITY_FACTORY_HPP__