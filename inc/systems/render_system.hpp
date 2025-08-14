#ifndef __SYSTEMS_RENDER_SYSTEM_HPP__
#define __SYSTEMS_RENDER_SYSTEM_HPP__

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>

#include <collision.hpp>
#include <font.hpp>
#include <memory>

#include <obstacle.hpp>
#include <pc.hpp>
#include <position.hpp>
#include <sprites/brick.hpp>
#include <sprites/player.hpp>
#include <systems/base_system.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Systems {

class RenderSystem : public Systems::BaseSystem {
public:
    RenderSystem(
        std::shared_ptr<sf::RenderWindow> win
    ) : 
        m_window( win )
    { 
        SPDLOG_DEBUG("RenderSystem()"); 
    }
    
    ~RenderSystem() { SPDLOG_DEBUG("~RenderSystem()"); } 
    
    void render()
    {
        using namespace Components;
        using namespace Sprites;
        m_window->clear();

            for( auto [entity, _pc, _pos]: m_position_updates.view<PlayableCharacter, Position>().each() ) {
                m_window->draw(  Player(_pos) );
            }

            for( auto [entity, _ob, _pos]: m_position_updates.view<Obstacle, Position>().each() ) {
                if( _ob.visible )
                    m_window->draw(  Brick(_pos) );
            }

            for( auto [entity, _collision]: m_position_updates.view<Collision>().each() ) {
                auto intersect = sf::RectangleShape(_collision.get()->size);
                intersect.setPosition(_collision.get()->position);
                intersect.setFillColor(sf::Color::Blue);
                m_window->draw(intersect);
            }

        m_window->display();
    }

    entt::reactive_mixin<entt::storage<void>> m_position_updates;
    entt::reactive_mixin<entt::storage<void>> m_collision_updates;

private:
    std::shared_ptr<sf::RenderWindow> m_window;

};

} // namespace ProceduralMaze::Systems

#endif // __SYSTEMS_RENDER_SYSTEM_HPP__