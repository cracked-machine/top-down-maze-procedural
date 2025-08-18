#ifndef __SYSTEMS_RENDER_SYSTEM_HPP__
#define __SYSTEMS_RENDER_SYSTEM_HPP__

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>

#include <Collision.hpp>
#include <Font.hpp>
#include <memory>

#include <Obstacle.hpp>
#include <PlayableCharacter.hpp>
#include <Position.hpp>
#include <ProcGen/RandomSystem.hpp>
#include <Settings.hpp>
#include <Sprites/Brick.hpp>
#include <Sprites/Player.hpp>
#include <System.hpp>
#include <Systems/BaseSystem.hpp>
#include <spdlog/spdlog.h>
#include <TileMap.hpp>
#include <XAxisHitBox.hpp>
#include <YAxisHitBox.hpp>

namespace ProceduralMaze::Sys {

class RenderSystem : public BaseSystem {
public:
    RenderSystem(
        std::shared_ptr<sf::RenderWindow> win
    ) : 
        m_window( win )
    { 
        using namespace ProceduralMaze::Settings;

        Sys::ProcGen::RandomSystem floortile_randsys({0, 4});

        floortile_randsys.gen({200, 97});
        auto tile_file = "res/floor_tiles_10x10.png";
        SPDLOG_INFO("size: {}", floortile_randsys.size());
        if (!m_floormap.load(tile_file, {10,10}, floortile_randsys.data(), 200, 97))
        {
            SPDLOG_CRITICAL("Unable to load tile map {}", tile_file);
        }

        SPDLOG_DEBUG("RenderSystem()"); 
    }
    
    ~RenderSystem() { SPDLOG_DEBUG("~RenderSystem()"); } 
    
    void render()
    {
        using namespace Sprites;
        auto f = Cmp::Font("res/tuffy.ttf");
        m_window->clear();
            
            m_floormap.setPosition({0, Settings::MAP_GRID_OFFSET.y * Sprites::Brick::FULLHEIGHT});
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

                m_local_view.setCenter(_pos);
            }

        m_window->display();

        // toggle between local view and global view (default)
        for( auto [entity, _sys]: 
            m_system_updates.view<Cmp::System>().each() ) 
        {
            if( _sys.local_view ) m_window->setView(m_local_view);
            else  m_window->setView(m_window->getDefaultView());
        }
        
    }

    entt::reactive_mixin<entt::storage<void>> m_position_updates;
    entt::reactive_mixin<entt::storage<void>> m_system_updates;
    sf::View m_local_view;
private:
    std::shared_ptr<sf::RenderWindow> m_window;
    Sprites::Containers::TileMap m_floormap;
};

} // namespace ProceduralMaze::Systems

#endif // __SYSTEMS_RENDER_SYSTEM_HPP__