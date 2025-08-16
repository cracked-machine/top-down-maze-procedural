#ifndef __SYSTEMS_RENDER_SYSTEM_HPP__
#define __SYSTEMS_RENDER_SYSTEM_HPP__

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
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
#include <xbb.hpp>
#include <ybb.hpp>

namespace ProceduralMaze::Sys {

class RenderSystem : public BaseSystem {
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
        using namespace Sprites;
        m_window->clear();

            for( auto [entity, _pc, _pos, _xbb, _ybb]: 
                m_position_updates.view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Xbb, Cmp::Ybb>().each() ) 
            {
                m_window->draw(  Player(_pos) );
                m_window->draw(_xbb.drawable());
                m_window->draw(_ybb.drawable());
            }

            for( auto [entity, _ob, _pos]: m_position_updates.view<Cmp::Obstacle, Cmp::Position>().each() ) {
                if( _ob.m_visible ) {
                    m_window->draw(  Brick(_pos) ); 
                }
            }

            for( auto [entity, _collision]: m_position_updates.view<Cmp::Collision>().each() ) {
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