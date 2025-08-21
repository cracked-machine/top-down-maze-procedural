#ifndef __SYSTEMS_RENDER_SYSTEM_HPP__
#define __SYSTEMS_RENDER_SYSTEM_HPP__

#include <BasicSprite.hpp>
#include <Neighbours.hpp>
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
        
        // local view
        m_local_view = sf::View( 
            { Settings::LOCAL_MAP_VIEW_SIZE.x * 0.5f, Settings::DISPLAY_SIZE.y * 0.5f}, 
            Settings::LOCAL_MAP_VIEW_SIZE 
        );
        m_local_view.setViewport( sf::FloatRect({0.f, 0.f}, {1.f, 1.f}) );

        // minimap view of entire level
        m_minimap_view = sf::View( 
            { Settings::MINI_MAP_VIEW_SIZE.x * 0.5f, Settings::DISPLAY_SIZE.y * 0.5f}, 
            Settings::MINI_MAP_VIEW_SIZE 
        );
        m_minimap_view.setViewport( sf::FloatRect({0.75f, 0.f}, {0.25f, 0.25f}) );

        SPDLOG_DEBUG("RenderSystem()"); 
    }
    
    ~RenderSystem() { SPDLOG_DEBUG("~RenderSystem()"); } 

    void render(entt::basic_registry<entt::entity> &reg)
    {
        using namespace Sprites;

        // main render begin
        m_window->clear();
        {
            // local view begin
            m_window->setView(m_local_view);
            {   
                render_floormap({0, Settings::MAP_GRID_OFFSET.y * Sprites::Brick::HEIGHT});
                render_bricks(reg);
                render_player();

                // update the minimap view center based on player position
                // reset the center if player is stuck
                for(auto [_ent, _sys]: m_system_updates.view<Cmp::System>().each()) {
                    if( _sys.player_stuck ) {
                        m_local_view.setCenter({Settings::LOCAL_MAP_VIEW_SIZE.x * 0.5f, Settings::DISPLAY_SIZE.y * 0.5f});
                        _sys.player_stuck = false;  
                    }
                    else {
                        for( auto [entity, _pc, _pos]: 
                            m_position_updates.view<Cmp::PlayableCharacter, Cmp::Position>().each() ) 
                        {
                            update_view_center(m_local_view, _pos);
                        }
                    }
                }                


            } 
            // local view end
            
            // minimap view begin
            m_window->setView(m_minimap_view);
            {
                render_floormap({0, Settings::MAP_GRID_OFFSET.y * Sprites::Brick::HEIGHT});
                render_bricks(reg);
                render_player();

                // update the minimap view center based on player position
                // reset the center if player is stuck
                for(auto [_ent, _sys]: m_system_updates.view<Cmp::System>().each()) {
                    if( _sys.player_stuck ) {
                        m_minimap_view.setCenter({Settings::MINI_MAP_VIEW_SIZE.x * 0.5f, Settings::MINI_MAP_VIEW_SIZE.y * 0.5f});
                        _sys.player_stuck = false;  
                    } else {
                        for( auto [entity, _pc, _pos]: 
                            m_position_updates.view<Cmp::PlayableCharacter, Cmp::Position>().each() ) 
                        {
                            update_view_center(m_minimap_view, _pos);
                        }
                    }
                }

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

    void render_bricks(entt::basic_registry<entt::entity> &reg)
    {
        bool show_obstacle_entity_id = false;
        for(auto [_ent, _sys]: m_system_updates.view<Cmp::System>().each()) {
            if( _sys.show_obstacle_entity_id ) show_obstacle_entity_id = true;
        }



        for( auto [entity, _ob, _pos, _nb]: 
            m_position_updates.view<Cmp::Obstacle, Cmp::Position, Cmp::Neighbours>().each() ) {
            
            // debug mode
            if( show_obstacle_entity_id )
            {
                if( _ob.m_enabled ) 
                {
                    auto generic_brick = Sprites::Brick(
                        _pos, 
                        Sprites::Brick::BEDROCK_FILLCOLOUR, 
                        Sprites::Brick::BEDROCK_LINECOLOUR
                    );
                    m_window->draw(generic_brick);
                }
                auto t = sf::Text(
                    m_font, 
                    std::to_string(entt::entt_traits<entt::entity>::to_entity(entity)),
                    Sprites::Brick::HALFHEIGHT
                );
                t.setPosition({_pos.x, _pos.y});
                t.setFillColor(sf::Color::Black);
                m_window->draw( t );
            }
            else 
            {

                if( _ob.m_type == Cmp::Obstacle::Type::BRICK && _ob.m_enabled) 
                {
                    m_object_sprite.setPosition(_pos);  
                    m_object_sprite.pick(_ob.m_tile_pick);
                    m_window->draw( m_object_sprite ); 
                }

            }

            
            if( _ob.m_armed )
            {
                // Draw a red square around the occupied brick
                sf::RectangleShape temp_square(Settings::OBSTACLE_SIZE_2F);
                temp_square.setPosition(_pos);
                temp_square.setFillColor(sf::Color::Transparent);
                temp_square.setOutlineColor(sf::Color::Red);
                temp_square.setOutlineThickness(1.f);
                m_window->draw(temp_square);

                for( auto [_dir, _nb_entt] : _nb) 
                {
                    sf::RectangleShape nb_square(Settings::OBSTACLE_SIZE_2F);
                    nb_square.setPosition(reg.get<Cmp::Position>( entt::entity(_nb_entt) ));                  
                    nb_square.setFillColor(sf::Color::Transparent);
                    nb_square.setOutlineColor(sf::Color::Blue); 
                    nb_square.setOutlineThickness(1.f); 
                    m_window->draw(nb_square);
                } 
            }            
                 
        }

        // we need a separate view for "bedrock" because it must not have any Neighbours component
        for( auto [entity, _ob, _pos]: 
            m_position_updates.view<Cmp::Obstacle, Cmp::Position>().each() ) {
            
            if( _ob.m_type == Cmp::Obstacle::Type::BEDROCK && _ob.m_enabled) 
            {
                m_border_sprite.setPosition(_pos);  
                m_border_sprite.pick(_ob.m_tile_pick);
                m_window->draw( m_border_sprite ); 
            }            
        }
    }

    void render_player()
    {
        for( auto [entity, _pc, _pos]: 
            m_position_updates.view<Cmp::PlayableCharacter, Cmp::Position>().each() ) 
        {
            m_player_sprite.setPosition({_pos.x, _pos.y - (Settings::PLAYER_SPRITE_SIZE.y / 2.f)});
            m_player_sprite.pick(0);
            m_window->draw(m_player_sprite);
        }
    }

    void update_view_center(sf::View &view, Cmp::Position &player_pos)
    {
        const float MAP_HALF_WIDTH = view.getSize().x * 0.5f;
        const float MAP_HALF_HEIGHT = view.getSize().y * 0.5f;
        
        // Calculate the maximum allowed camera positions
        float maxX = Settings::DISPLAY_SIZE.x - MAP_HALF_WIDTH;
        float maxY = Settings::DISPLAY_SIZE.y - MAP_HALF_HEIGHT;
        
        // Calculate new camera position
        float newX = std::clamp(player_pos.x, MAP_HALF_WIDTH, maxX);
        float newY = std::clamp(player_pos.y, MAP_HALF_HEIGHT, maxY);
        
        // Smoothly interpolate to the new position
        sf::Vector2f currentCenter = view.getCenter();
        float smoothFactor = 0.1f; // Adjust this value to change how quickly the camera follows
        
        view.setCenter({
            currentCenter.x + (newX - currentCenter.x) * smoothFactor,
            currentCenter.y + (newY - currentCenter.y) * smoothFactor
        });
    }
    
    entt::reactive_mixin<entt::storage<void>> m_position_updates;
    entt::reactive_mixin<entt::storage<void>> m_system_updates;

    sf::View m_local_view;
    sf::View m_minimap_view;
private:
    std::shared_ptr<sf::RenderWindow> m_window;
    Sprites::Containers::TileMap m_floormap;
    Sprites::MultiSprite m_object_sprite{
        Settings::OBJECT_TILESET_PATH,
        Settings::OBJECT_TILE_POOL,
        Settings::OBSTACLE_SIZE
    };

    Sprites::MultiSprite m_border_sprite{
        Settings::BORDER_TILESET_PATH,
        Settings::BORDER_TILE_POOL,
        Settings::OBSTACLE_SIZE
    };

    Sprites::MultiSprite m_player_sprite{
        Settings::PLAYER_TILESET_PATH,
        Settings::PLAYER_TILE_POOL,
        Settings::PLAYER_SPRITE_SIZE
    };

    Cmp::Font m_font = Cmp::Font("res/tuffy.ttf");
// 
};

} // namespace ProceduralMaze::Systems

#endif // __SYSTEMS_RENDER_SYSTEM_HPP__