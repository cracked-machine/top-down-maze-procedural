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

    static void add_bedrock_entity(entt::basic_registry<entt::entity> &m_reg, const sf::Vector2f &pos)
    {
        auto entity = m_reg.create();
        m_reg.emplace<Cmp::Position>(entity, pos); 
        m_reg.emplace<Cmp::Obstacle>(entity, Cmp::Obstacle::Type::BEDROCK, true, true );
    }

    static void add_border(entt::basic_registry<entt::entity> &m_reg)
    {
        using namespace ProceduralMaze::Settings;

        for(float x = MAP_GRID_OFFSET.x - (Sprites::Brick::FULLWIDTH * 3) ; x < DISPLAY_SIZE.x; x += Sprites::Brick::FULLWIDTH)
        {
            // top edge
            EntityFactory::add_bedrock_entity(m_reg, {
                x, 
                MAP_GRID_OFFSET.y - Sprites::Brick::FULLHEIGHT
            });
            // bottom edge
            EntityFactory::add_bedrock_entity(m_reg, {
                x, 
                MAP_GRID_OFFSET.y + (MAP_GRID_SIZE.y * (Sprites::Brick::HEIGHT + Sprites::Brick::LINETHICKNESS) ) + Sprites::Brick::LINETHICKNESS
            });
        }
        for( float y = MAP_GRID_OFFSET.y; y < DISPLAY_SIZE.y; y += Sprites::Brick::FULLHEIGHT)
        {
            // left edge 
            EntityFactory::add_bedrock_entity( m_reg, {0, y} );
            // right edge
            EntityFactory::add_bedrock_entity( m_reg,{static_cast<float>(DISPLAY_SIZE.x), y} );
        }

    }
    
    static void add_player_entity(entt::basic_registry<entt::entity> &m_reg, const sf::Vector2f &pos)
    {
        auto entity = m_reg.create();
        m_reg.emplace<Cmp::Position>(entity, pos); 
        m_reg.emplace<Cmp::PlayableCharacter>(entity );
        m_reg.emplace<Cmp::Xbb>(
            entity, 
            sf::Vector2f(pos.x - (Cmp::Xbb::EXTRA_WIDTH / 2), pos.y), 
            sf::Vector2f(Sprites::Player::WIDTH + Cmp::Xbb::EXTRA_WIDTH, Sprites::Player::HEIGHT) 
        );
        m_reg.emplace<Cmp::Ybb>(
            entity, 
            sf::Vector2f(pos.x, pos.y - (Cmp::Ybb::EXTRA_HEIGHT / 2)), 
            sf::Vector2f(Sprites::Player::WIDTH, Sprites::Player::HEIGHT + Cmp::Ybb::EXTRA_HEIGHT) 
        );
    }

private:
    EntityFactory() {}
};

} // namespace ProceduralMaze

#endif // __ENTITY_FACTORY_HPP__