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
#include <procedural_generation/RandomSystem.hpp>
#include <settings.hpp>
#include <sprites/brick.hpp>
#include <sprites/player.hpp>
#include <system.hpp>
#include <systems/base_system.hpp>
#include <spdlog/spdlog.h>
#include <tile_map.hpp>
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
        using namespace ProceduralMaze::Settings;
        Sys::ProcGen::RandomSystem floortile_randsys(
            MAP_GRID_SIZE, 
            ProceduralMaze::Settings::MAP_GRID_OFFSET, 
            {0, 4});

        floortile_randsys.gen(0);
        auto tile_file = "res/floor_tiles_10x10.png";
        // for(auto v: floortile_randsys) { SPDLOG_INFO(v); }
        SPDLOG_INFO("size: {}", floortile_randsys.size());
        if (!m_floormap.load(tile_file, {10,10}, floortile_randsys.data(), MAP_GRID_SIZE.x, MAP_GRID_SIZE.y))
        {
            SPDLOG_CRITICAL("Unable to load tile map {}", tile_file);
            // std::terminate();
        }
        SPDLOG_DEBUG("RenderSystem()"); 
    }
    
    ~RenderSystem() { SPDLOG_DEBUG("~RenderSystem()"); } 
    
    void render()
    {
        using namespace Sprites;
        auto f = Cmp::Font("res/tuffy.ttf");
        m_window->clear();
            
            m_window->draw(m_floormap);

            // bricks
            for( auto [entity, _ob, _pos]: m_position_updates.view<Cmp::Obstacle, Cmp::Position>().each() ) {
                
                if( not _ob.m_enabled ) { continue; }
                
                if( _ob.m_type == Cmp::Obstacle::Type::BRICK ) 
                {
                    m_window->draw( Brick(_pos, Sprites::Brick::BRICK_FILLCOLOUR, Sprites::Brick::BRICK_LINECOLOUR) ); 
                }
                else
                {
                    m_window->draw( Brick(_pos, Sprites::Brick::BEDROCK_FILLCOLOUR, Sprites::Brick::BEDROCK_LINECOLOUR) ); 
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

                m_current_view.setCenter(_pos);
            }

        m_window->display();

        for( auto [entity, _sys]: 
            m_system_updates.view<Cmp::System>().each() ) 
        {
            SPDLOG_INFO("read _sys.local_view as {}", _sys.local_view);
            if( _sys.local_view ) m_window->setView(m_current_view);
            else  m_window->setView(m_window->getDefaultView());
        }
        
    }

    entt::reactive_mixin<entt::storage<void>> m_position_updates;
    entt::reactive_mixin<entt::storage<void>> m_system_updates;
    sf::View m_current_view;
private:
    std::shared_ptr<sf::RenderWindow> m_window;

    Sprites::Containers::TileMap m_floormap;
    

};

} // namespace ProceduralMaze::Systems

#endif // __SYSTEMS_RENDER_SYSTEM_HPP__