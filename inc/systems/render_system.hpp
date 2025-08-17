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
        auto f = Cmp::Font("res/tuffy.ttf");
        m_window->clear();

            // bricks
            for( auto [entity, _ob, _pos]: m_position_updates.view<Cmp::Obstacle, Cmp::Position>().each() ) {
                
                if( _ob.m_enabled ) {
                    if( _ob.m_type == Cmp::Obstacle::Type::BRICK ) {
                        m_window->draw( Brick(_pos, Sprites::Brick::BRICK_FILLCOLOUR, Sprites::Brick::BRICK_LINECOLOUR) ); 
                    }
                    else
                    {
                        m_window->draw( Brick(_pos, Sprites::Brick::BEDROCK_FILLCOLOUR, Sprites::Brick::BEDROCK_LINECOLOUR) ); 
                    }
                }
#ifdef SHOW_BRICK_ENTITY_ID
                auto t = sf::Text(
                    f, 
                    std::to_string(entt::entt_traits<entt::entity>::to_entity(entity)),
                    Sprites::Brick::HALFHEIGHT
                );
                t.setPosition({_pos.x, _pos.y});
                m_window->draw( t );
#endif // SHOW_BRICK_ENTITY_ID
            }

            // player           
            for( auto [entity, _pc, _pos, _xbb, _ybb]: 
                m_position_updates.view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Xbb, Cmp::Ybb>().each() ) 
            {
                m_window->draw(  Player(_pos) );
#ifdef SHOW_PLAYER_BOUNDING_BOX
                m_window->draw(_xbb.drawable());
                m_window->draw(_ybb.drawable());
#endif
            }

        m_window->display();
    }

    entt::reactive_mixin<entt::storage<void>> m_position_updates;

private:
    std::shared_ptr<sf::RenderWindow> m_window;

};

} // namespace ProceduralMaze::Systems

#endif // __SYSTEMS_RENDER_SYSTEM_HPP__