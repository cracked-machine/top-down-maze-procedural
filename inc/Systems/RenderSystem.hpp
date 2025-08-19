#ifndef __SYSTEMS_RENDER_SYSTEM_HPP__
#define __SYSTEMS_RENDER_SYSTEM_HPP__

#include <BasicSprite.hpp>
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
#include <Settings.hpp>
#include <Sprites/Brick.hpp>
#include <Sprites/Player.hpp>
#include <System.hpp>
#include <Systems/BaseSystem.hpp>
#include <spdlog/spdlog.h>
#include <TileMap.hpp>
#include <XAxisHitBox.hpp>
#include <YAxisHitBox.hpp>
#include <MultiSprite.hpp>

namespace ProceduralMaze::Sys {

class RenderSystem : public BaseSystem {
public:
    RenderSystem(
        std::shared_ptr<sf::RenderWindow> win
    ) : 
        m_window( win )
    { 
        using namespace ProceduralMaze::Settings;

        // setup the floortile background texture
        // TODO move this to a separate system
        Cmp::Random floortile_picker(0, FLOOR_TILE_POOL.size() - 1);
        std::vector<uint32_t> floortile_choices;
        for(int x = 0; x < 200; x++) 
            for(int y = 0; y < 98; y++) 
                floortile_choices.push_back(FLOOR_TILE_POOL[floortile_picker.gen()]); 
        
        auto tile_file = "res/kenney_tiny-dungeon/Tilemap/tilemap_packed.png";
        if (!m_floormap.load(tile_file, {16,16}, floortile_choices.data(), 200, 98))
            SPDLOG_CRITICAL("Unable to load tile map {}", tile_file);
        

        SPDLOG_DEBUG("RenderSystem()"); 
    }
    
    ~RenderSystem() { SPDLOG_DEBUG("~RenderSystem()"); } 

    void render()
    {
        using namespace Sprites;
        auto f = Cmp::Font("res/tuffy.ttf");

        // main render begin
        m_window->clear();
        {
            // local view begin
            m_window->setView(m_local_view);
            {   
                render_floormap({0, Settings::MAP_GRID_OFFSET.y * Sprites::Brick::HEIGHT});
                render_bricks();
                render_player(m_local_view);
            } 
            // local view end
            
            // minimap view begin
            m_window->setView(m_minimap_view);
            {
                render_floormap({0, Settings::MAP_GRID_OFFSET.y * Sprites::Brick::HEIGHT});
                render_bricks();
                render_player(m_minimap_view);
            } 
            // minimap view end

            // UI Overlays begin
            m_window->setView(m_window->getDefaultView());
            {  
                auto minimap_border = sf::RectangleShape(
                    {Settings::MINI_MAP_VIEW_SIZE.x, Settings::MINI_MAP_VIEW_SIZE.y}
                );
                minimap_border.setPosition(
                    {Settings::DISPLAY_SIZE.x - Settings::MINI_MAP_VIEW_SIZE.x, 0.f}
                );
                minimap_border.setFillColor(sf::Color::Transparent);
                minimap_border.setOutlineColor(sf::Color::White);
                minimap_border.setOutlineThickness(2.f);
                m_window->draw(minimap_border);       
            } 
            // UI overlays end

        } 
        m_window->display();
        // main render end
    }

    void render_floormap(const sf::Vector2f &offset = {0.f, 0.f})
    {
        m_floormap.setPosition(offset);
        m_window->draw(m_floormap);
    }

    void render_bricks()
    {
        for( auto [entity, _ob, _pos]: m_position_updates.view<Cmp::Obstacle, Cmp::Position>().each() ) {
            if( not _ob.m_enabled ) { continue; }
            
            if( _ob.m_type == Cmp::Obstacle::Type::BRICK ) 
            {
                m_wall_sprite.setPosition(_pos);  
                m_wall_sprite.pick(_ob.m_tile_pick);
                m_window->draw( m_wall_sprite ); 
            }
            else
            {
                // m_window->draw( Sprites::Brick(_pos, Sprites::Brick::BEDROCK_FILLCOLOUR, Sprites::Brick::BEDROCK_LINECOLOUR) ); 
                m_border_sprite.setPosition(_pos);  
                m_border_sprite.pick(_ob.m_tile_pick);
                m_window->draw( m_border_sprite ); 
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
    }

    void render_player(sf::View &view)
    {
        for( auto [entity, _pc, _pos, _xbb, _ybb]: 
            m_position_updates.view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Xbb, Cmp::Ybb>().each() ) 
        {
            m_player_sprite.setPosition(_pos);
            m_player_sprite.pick(0);
            m_window->draw(m_player_sprite);
            // m_window->draw( Sprites::Player(_pos) );

            #ifdef SHOW_PLAYER_HIT_BOXES
            m_window->draw(_xbb.drawable());
            m_window->draw(_ybb.drawable());
            #endif
           
            update_view_center(view, _pos);
        }
    }

    void update_view_center(sf::View &view, Cmp::Position &player_pos)
    {
            const float MAP_HALF_WIDTH = view.getSize().x * 0.5f;
            const float MAP_HALF_HEIGHT = view.getSize().y * 0.5f;

            if( ( player_pos.x > MAP_HALF_WIDTH && player_pos.x <  Settings::DISPLAY_SIZE.x - MAP_HALF_WIDTH )
            ) {
                view.setCenter({player_pos.x, view.getCenter().y});
            }
            if( player_pos.y > MAP_HALF_HEIGHT && player_pos.y < Settings::DISPLAY_SIZE.y - MAP_HALF_HEIGHT)
            {
                view.setCenter({view.getCenter().x, player_pos.y});
            }            
    }
    
    entt::reactive_mixin<entt::storage<void>> m_position_updates;
    sf::View m_local_view;
    sf::View m_minimap_view;
private:
    std::shared_ptr<sf::RenderWindow> m_window;
    Sprites::Containers::TileMap m_floormap;
    // Sprites::BasicSprite wall_sprite{"res/wall.png"};
    // Sprites::BasicSprite wall_sprite{"res/kenney_tiny-dungeon/Tiles/tile_0041.png"};
    // Sprites::BasicSprite border_sprite{"res/kenney_tiny-dungeon/Tiles/tile_0040.png"};
    Sprites::MultiSprite m_wall_sprite{
        Settings::WALL_TILESET_PATH,
        Settings::WALL_TILE_POOL
    };

    Sprites::MultiSprite m_border_sprite{
        Settings::BORDER_TILESET_PATH,
        Settings::BORDER_TILE_POOL
    };

    Sprites::MultiSprite m_player_sprite{
        Settings::PLAYER_TILESET_PATH,
        Settings::PLAYER_TILE_POOL,
        Settings::PLAYER_SIZE
    };
// 
};

} // namespace ProceduralMaze::Systems

#endif // __SYSTEMS_RENDER_SYSTEM_HPP__