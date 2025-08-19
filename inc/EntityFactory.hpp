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

    
    static void add_player_entity(entt::basic_registry<entt::entity> &m_reg, const sf::Vector2f &pos)
    {
        auto entity = m_reg.create();
        m_reg.emplace<Cmp::Position>(entity, pos); 
        m_reg.emplace<Cmp::PlayableCharacter>(entity );
        m_reg.emplace<Cmp::Xbb>(
            entity, 
            sf::Vector2f(pos.x - (Cmp::Xbb::EXTRA_WIDTH / 2), pos.y), 
            sf::Vector2f((Settings::PLAYER_SIZE.x - 2) + Cmp::Xbb::EXTRA_WIDTH, (Settings::PLAYER_SIZE.y - 2)) 
        );
        m_reg.emplace<Cmp::Ybb>(
            entity, 
            sf::Vector2f(pos.x, pos.y - (Cmp::Ybb::EXTRA_HEIGHT / 2)), 
            sf::Vector2f((Settings::PLAYER_SIZE.x - 2), (Settings::PLAYER_SIZE.y - 2) + Cmp::Ybb::EXTRA_HEIGHT) 
        );
    }

private:
    EntityFactory() {}
};

} // namespace ProceduralMaze

#endif // __ENTITY_FACTORY_HPP__