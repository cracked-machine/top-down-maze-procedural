#ifndef __SYSTEMS_RENDER_SYSTEM_HPP__
#define __SYSTEMS_RENDER_SYSTEM_HPP__

#include <Components/Armed.hpp>
#include <Components/Direction.hpp>
#include <Components/Font.hpp>
#include <Components/Loot.hpp>
#include <Components/Neighbours.hpp>
#include <Components/NPC.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PlayerDistance.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Position.hpp>
#include <Components/System.hpp>
#include <Components/WaterLevel.hpp>
#include <NPCScanBounds.hpp>
#include <PCDetectionBounds.hpp>
#include <Sprites/BasicSprite.hpp>
#include <Sprites/FloodWaterShader.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Sprites/TileMap.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/PathFindSystem.hpp>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

#include <SFML/System/Vector2.hpp>
#include <entt/entity/fwd.hpp>

#include <exception>
#include <memory>
#include <sstream>

#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys {

class RenderSystem : public BaseSystem {
public:
    RenderSystem(
        std::shared_ptr<entt::basic_registry<entt::entity>> reg,
        std::shared_ptr<sf::RenderWindow> win,
        std::shared_ptr<Sys::PathFindSystem> path_find_sys
    ) : 
        BaseSystem( reg ),
        m_window( win ),
        m_path_find_sys( path_find_sys )
    { 

        SPDLOG_INFO("RenderSystem initialisation starting..."); 
        
        // init local view dimensions
        m_local_view = sf::View( 
            { LOCAL_MAP_VIEW_SIZE.x * 0.5f, LOCAL_MAP_VIEW_SIZE.y * 0.5f}, 
            LOCAL_MAP_VIEW_SIZE 
        );
        m_local_view.setViewport( sf::FloatRect({0.f, 0.f}, {1.f, 1.f}) );

        // init minimap view dimensions
        m_minimap_view = sf::View( 
            { MINI_MAP_VIEW_SIZE.x * 0.5f, MINI_MAP_VIEW_SIZE.y * 0.5f}, 
            MINI_MAP_VIEW_SIZE 
        );
        m_minimap_view.setViewport( sf::FloatRect({0.75f, 0.f}, {0.25f, 0.25f}) );

        // we should ensure these MultiSprites are initialized before continuing
        if( not m_rock_ms ) { SPDLOG_CRITICAL("Unable to get ROCK multisprite from SpriteFactory"); std::get_terminate(); }
        if( not m_pot_ms ) { SPDLOG_CRITICAL("Unable to get POT multisprite from SpriteFactory"); std::get_terminate(); }
        if( not m_bone_ms ) { SPDLOG_CRITICAL("Unable to get BONE multisprite from SpriteFactory"); std::get_terminate(); }
        if( not m_player_ms ) { SPDLOG_CRITICAL("Unable to get PLAYER multisprite from SpriteFactory"); std::get_terminate(); }
        if( not m_bomb_ms ) { SPDLOG_CRITICAL("Unable to get BOMB multisprite from SpriteFactory"); std::get_terminate(); }
        if( not m_detonation_ms ) { SPDLOG_CRITICAL("Unable to get DETONATION multisprite from SpriteFactory"); std::get_terminate(); }
        if( not m_wall_ms ) { SPDLOG_CRITICAL("Unable to get WALL multisprite from SpriteFactory"); std::get_terminate(); }
        if( not m_npc_ms ) { SPDLOG_CRITICAL("Unable to get NPC multisprite from SpriteFactory"); std::get_terminate(); }

        if( not m_extra_health_ms ) { SPDLOG_CRITICAL("Unable to get EXTRA_HEALTH multisprite from SpriteFactory"); std::get_terminate(); }
        if( not m_extra_bombs_ms ) { SPDLOG_CRITICAL("Unable to get EXTRA_BOMBS multisprite from SpriteFactory"); std::get_terminate(); }
        if( not m_infinite_bombs_ms ) { SPDLOG_CRITICAL("Unable to get INFINI_BOMBS multisprite from SpriteFactory"); std::get_terminate(); }
        if( not m_chain_bombs_ms ) { SPDLOG_CRITICAL("Unable to get CHAIN_BOMBS multisprite from SpriteFactory"); std::get_terminate(); }
        if( not m_lower_water_ms ) { SPDLOG_CRITICAL("Unable to get LOWER_WATER multisprite from SpriteFactory"); std::get_terminate(); }

        // initWaterShader();
        SPDLOG_INFO("RenderSystem initialisation finished"); 
    }
    
    ~RenderSystem() { SPDLOG_DEBUG("~RenderSystem()"); } 



    void render_menu()
    {
        // main render begin
        m_window->clear();
        {
            sf::Text title_text(m_font, "Procedural Maze Game", 96);
            title_text.setFillColor(sf::Color::White);
            title_text.setPosition({DISPLAY_SIZE.x / 4.f, 100.f});
            m_window->draw(title_text);

            sf::Text start_text(m_font, "Press <Enter> key to start", 48);
            start_text.setFillColor(sf::Color::White);
            start_text.setPosition({DISPLAY_SIZE.x / 4.f, 300.f});
            m_window->draw(start_text);

            sf::Text quit_text(m_font, "Press <Q> key to quit", 48);
            quit_text.setFillColor(sf::Color::White);
            quit_text.setPosition({DISPLAY_SIZE.x / 4.f, 350.f});
            m_window->draw(quit_text);
        }
        m_window->display();
        // main render end
    }

    void render_game()
    {
        using namespace Sprites;

        for(auto [_ent, _sys]: m_system_updates.view<Cmp::System>().each()) {
            m_show_path_distances = _sys.show_path_distances;
            m_show_armed_obstacles = _sys.show_armed_obstacles;
        }

        // main render begin
        m_window->clear();
        {
            // local view begin - this shows only a `LOCAL_MAP_VIEW_SIZE` of the game world
            m_window->setView(m_local_view);
            {   
                render_floormap({0, MAP_GRID_OFFSET.y * m_sprite_factory->DEFAULT_SPRITE_SIZE.y});
                render_obstacles();
                render_armed();
                render_loot();
                render_walls();
                render_player();
                render_npc();
                render_flood_waters();
                render_player_distances_on_npc();
                // render_npc_distances_on_obstacles();      
                render_player_distances_on_obstacles();          

                // move the local view position to equal the player position
                // reset the center if player is stuck
                for(auto [_ent, _sys]: m_system_updates.view<Cmp::System>().each()) {
                    if( _sys.player_stuck ) {
                        m_local_view.setCenter({LOCAL_MAP_VIEW_SIZE.x * 0.5f, DISPLAY_SIZE.y * 0.5f});
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

            // minimap view begin - this show a quarter of the game world but in a much smaller scale
            m_window->setView(m_minimap_view);
            {
                render_floormap({0, MAP_GRID_OFFSET.y * m_sprite_factory->DEFAULT_SPRITE_SIZE.y});
                render_obstacles();
                render_armed();
                render_loot();
                render_walls();
                render_player();
                render_npc();
                render_flood_waters();

                // update the minimap view center based on player position
                // reset the center if player is stuck
                for(auto [_ent, _sys]: m_system_updates.view<Cmp::System>().each()) {
                    if( _sys.player_stuck ) {
                        m_minimap_view.setCenter({MINI_MAP_VIEW_SIZE.x * 0.5f, MINI_MAP_VIEW_SIZE.y * 0.5f});
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

            // UI Overlays begin (these will always be displayed no matter where the player moves)
            m_window->setView(m_window->getDefaultView());
            {  
                auto minimap_border = sf::RectangleShape(
                    {MINI_MAP_VIEW_SIZE.x, MINI_MAP_VIEW_SIZE.y}
                );
                minimap_border.setPosition(
                    {DISPLAY_SIZE.x - MINI_MAP_VIEW_SIZE.x, 0.f}
                );
                minimap_border.setFillColor(sf::Color::Transparent);
                minimap_border.setOutlineColor(sf::Color::White);
                minimap_border.setOutlineThickness(2.f);
                m_window->draw(minimap_border);           

                for(auto [_entt, _pc]: m_reg->view<Cmp::PlayableCharacter>().each()) {
                    render_health_overlay(_pc.health, {40.f, 20.f},  {200.f, 20.f});
                    render_bomb_overlay(_pc.bomb_inventory, {40.f, 120.f});
                    render_bomb_radius_overlay(_pc.blast_radius, {40.f, 150.f});
                }

                for(auto [_entt, water_level]: m_reg->view<Cmp::WaterLevel>().each()) {
                    render_water_level_meter_overlay(water_level.m_level, {40.f, 70.f},  {200.f, 20.f});
                }

                render_entt_distance_set_overlay({40.f, 300.f});

            } 
            // UI Overlays end

        } 
        m_window->display();
        // main render end
    }

    void render_paused()
    {
        // main render begin
        m_window->clear();
        {
            sf::Text title_text(m_font, "Paused", 96);
            title_text.setFillColor(sf::Color::White);
            title_text.setPosition({DISPLAY_SIZE.x / 4.f, 100.f});
            m_window->draw(title_text);

            sf::Text start_text(m_font, "Press P to continue", 48);
            start_text.setFillColor(sf::Color::White);
            start_text.setPosition({DISPLAY_SIZE.x / 4.f, 200.f});
            m_window->draw(start_text);
        }
        m_window->display();
        // main render end
    }


    void render_defeat_screen()
    {
        // main render begin
        m_window->clear();
        {
            sf::Text title_text(m_font, "You died!", 96);
            title_text.setFillColor(sf::Color::White);
            title_text.setPosition({DISPLAY_SIZE.x / 4.f, 100.f});
            m_window->draw(title_text);

            sf::Text start_text(m_font, "Press <R> key to continue", 48);
            start_text.setFillColor(sf::Color::White);
            start_text.setPosition({DISPLAY_SIZE.x / 4.f, 200.f});
            m_window->draw(start_text);
        }
        m_window->display();
        // main render end
    }

    void render_victory_screen()
    {
        // main render begin
        m_window->clear();
        {
            sf::Text title_text(m_font, "You won!", 96);
            title_text.setFillColor(sf::Color::White);
            title_text.setPosition({DISPLAY_SIZE.x / 4.f, 100.f});
            m_window->draw(title_text);

            sf::Text start_text(m_font, "Press <R> key to continue", 48);
            start_text.setFillColor(sf::Color::White);
            start_text.setPosition({DISPLAY_SIZE.x / 4.f, 200.f});
            m_window->draw(start_text);
        }
        m_window->display();
        // main render end
    }

    entt::reactive_mixin<entt::storage<void>> m_position_updates;
    entt::reactive_mixin<entt::storage<void>> m_system_updates;
    entt::reactive_mixin<entt::storage<void>> m_flood_updates;

    sf::View m_local_view;
    sf::View m_minimap_view;

    // creates and manages MultiSprite resources
    std::shared_ptr<Sprites::SpriteFactory> m_sprite_factory = std::make_shared<Sprites::SpriteFactory>();

private:
    /////////////////////////
    // GAME RENDER FUNCTIONS
    /////////////////////////

    void render_floormap(const sf::Vector2f &offset = {0.f, 0.f})
    {
        m_floormap.setPosition(offset);
        m_window->draw(m_floormap);
    }

    void render_obstacles() {

        // Group similar draw operations to reduce state changes
        std::vector<std::pair<sf::Vector2f, int>> rockPositions;
        std::vector<std::pair<sf::Vector2f, int>> potPositions;
        std::vector<std::pair<sf::Vector2f, int>> bonePositions;
        std::vector<std::pair<sf::Vector2f, int>> npcPositions;
        std::vector<std::pair<sf::Vector2f, Sprites::SpriteFactory::Type>> disabledPositions;
        std::vector<sf::Vector2f> detonationPositions;

        // Collect all positions first instead of drawing immediately
        for(auto [entity, _ob, _pos, _ob_nb_list]: 
            m_position_updates.view<Cmp::Obstacle, Cmp::Position, Cmp::Neighbours>().each()) {
            
            if(_ob.m_enabled) {
                switch(_ob.m_type) {
                    case Sprites::SpriteFactory::Type::ROCK:
                        rockPositions.emplace_back(_pos, _ob.m_tile_index);
                        break;
                    case Sprites::SpriteFactory::Type::POT:
                        potPositions.emplace_back(_pos, _ob.m_tile_index);
                        break;
                    case Sprites::SpriteFactory::Type::BONES:
                        bonePositions.emplace_back(_pos, _ob.m_tile_index);
                        break;
                    default:
                        break;
                }
            }
            // else {
            //     disabledPositions.emplace_back(_pos, _ob.m_type);
            // }
            if(_ob.m_broken) {
                detonationPositions.push_back(_pos);
            }  
        }

        // Now draw each type in batches
        for(const auto& [pos, idx]: rockPositions) {
            m_rock_ms->pick(idx, "Obstacle");
            m_rock_ms->setPosition(pos);
            m_window->draw(*m_rock_ms);
        }
        
        for(const auto& [pos, idx]: potPositions) {
            m_pot_ms->pick(idx, "Obstacle");
            m_pot_ms->setPosition(pos);
            m_window->draw(*m_pot_ms);
        }
        
        for(const auto& [pos, idx]: bonePositions) {
            m_bone_ms->pick(idx, "Obstacle");
            m_bone_ms->setPosition(pos);
            m_window->draw(*m_bone_ms);
        }

        // "empty" sprite for detonated objects
        for(const auto& pos: detonationPositions) {
            m_detonation_ms->setPosition(pos);
            m_detonation_ms->pick(0, "Detonated");
            m_window->draw(*m_detonation_ms);
        }

        // for(const auto& [pos, type]: disabledPositions) {
        //     sf::Text text(m_font, "", 12);
        //     text.setString(m_sprite_factory->get_metadata_type_string(type));
        //     text.setPosition(pos);
        //     m_window->draw(text);

        //     sf::RectangleShape temp_square(sf::Vector2f{m_sprite_factory->DEFAULT_SPRITE_SIZE});
        //     temp_square.setPosition(pos);
        //     temp_square.setFillColor(sf::Color::Transparent);
        //     temp_square.setOutlineColor(sf::Color::Red);
        //     temp_square.setOutlineThickness(1.f);
        //     m_window->draw(temp_square);
        // }
    }

    void render_armed()
    {
        // render armed obstacles with debug outlines
        auto all_armed_obstacles_view = m_reg->view<Cmp::Obstacle, Cmp::Armed, Cmp::Position>();
        for( auto [entity, obstacle_cmp, armed_cmp, pos_cmp]: all_armed_obstacles_view.each() ) 
        {
            if(armed_cmp.m_display_bomb_sprite) 
            {
                m_bomb_ms->pick(0, "Bomb");
                m_bomb_ms->setPosition(pos_cmp);
                m_window->draw(*m_bomb_ms);
            }
            
            sf::RectangleShape temp_square(sf::Vector2f{m_sprite_factory->DEFAULT_SPRITE_SIZE});
            temp_square.setPosition(pos_cmp);
            temp_square.setOutlineColor(sf::Color::Transparent);
            temp_square.setFillColor(sf::Color::Transparent);
            if(armed_cmp.getElapsedWarningTime() > armed_cmp.m_warning_delay) 
            {
                temp_square.setOutlineColor(armed_cmp.m_armed_color );
                temp_square.setFillColor(armed_cmp.m_armed_color );
            }
            temp_square.setOutlineThickness(1.f);
            m_window->draw(temp_square);
            
            // debug - F4
            if (m_show_armed_obstacles)
            {
                sf::Text text(m_font, "", 12);
                text.setString(std::to_string(armed_cmp.m_index));
                text.setPosition(pos_cmp);
                m_window->draw(text);
            }
        }

    }

    void render_loot()
    {
        auto loot_view = m_position_updates.view<Cmp::Obstacle, Cmp::Loot, Cmp::Position>();
        for( auto [entity, obstacles, loot, position] : loot_view.each() ) 
        {    
            switch( loot.m_type ) 
            {
                case ProceduralMaze::Sprites::SpriteFactory::Type::EXTRA_HEALTH:
                    m_extra_health_ms->setPosition(position);
                    m_extra_health_ms->pick(loot.m_tile_index, "EXTRA_HEALTH");
                    m_window->draw(*m_extra_health_ms);
                    break;
                case ProceduralMaze::Sprites::SpriteFactory::Type::EXTRA_BOMBS:
                    m_extra_bombs_ms->setPosition(position);
                    m_extra_bombs_ms->pick(loot.m_tile_index, "EXTRA_BOMBS");
                    m_window->draw(*m_extra_bombs_ms);
                    break;
                case ProceduralMaze::Sprites::SpriteFactory::Type::INFINI_BOMBS:
                    m_infinite_bombs_ms->setPosition(position);
                    m_infinite_bombs_ms->pick(loot.m_tile_index, "INFINI_BOMBS");
                    m_window->draw(*m_infinite_bombs_ms);
                    break;                        
                case ProceduralMaze::Sprites::SpriteFactory::Type::CHAIN_BOMBS:
                    m_chain_bombs_ms->setPosition(position);
                    m_chain_bombs_ms->pick(loot.m_tile_index, "CHAIN_BOMBS");
                    m_window->draw(*m_chain_bombs_ms);
                    break;                        
                case ProceduralMaze::Sprites::SpriteFactory::Type::LOWER_WATER:
                    m_lower_water_ms->setPosition(position);
                    m_lower_water_ms->pick(loot.m_tile_index, "LOWER_WATER");
                    m_window->draw(*m_lower_water_ms);
                    break;                        
                default:
                    break;
            }
        }
    }

    void render_walls()
    {
        // Render textures for "WALL" entities - filtered out because they don't own neighbour components
        for( auto [entity, _ob, _pos]: 
            m_position_updates.view<Cmp::Obstacle, Cmp::Position>(entt::exclude<Cmp::Neighbours>).each() ) {

                m_wall_ms->pick(_ob.m_tile_index, "wall");
                m_wall_ms->setPosition(_pos);
                m_window->draw(*m_wall_ms);
            
        }
    }

    void render_player()
    {
        for( auto [entity, player, position, direction, pc_detection_bounds]: 
            m_position_updates.view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Direction, Cmp::PCDetectionBounds>().each() ) 
        {

            // flip and x-axis offset the sprite depending on the direction
            if( direction.x == 1 )
            {
                direction.x_scale = 1.f;
                direction.x_offset = 0.f;
            }
            else if ( direction.x == -1 ) 
            {
                direction.x_scale = -1.f;
                direction.x_offset = m_sprite_factory->DEFAULT_SPRITE_SIZE.x;
            }
            else 
            {
                direction.x_scale =  direction.x_scale; // keep last known direction
                direction.x_offset = direction.x_offset;
            }

            m_player_ms->setScale({direction.x_scale, 1.f});
            m_player_ms->setPosition({position.x + direction.x_offset, position.y});


            m_player_ms->pick(0, "player");
            m_window->draw(*m_player_ms);
            if (m_show_path_distances)
            {
                sf::RectangleShape pc_square(pc_detection_bounds.size());
                pc_square.setFillColor(sf::Color::Transparent);
                pc_square.setOutlineColor(sf::Color::Green);
                pc_square.setOutlineThickness(1.f);
                pc_square.setPosition(pc_detection_bounds.position());
                m_window->draw(pc_square);
            }
        }
    }

    void render_npc()
    {
        for( auto [entity, npc, pos, npc_scan_bounds, direction]: 
            m_position_updates.view<Cmp::NPC, Cmp::Position, Cmp::NPCScanBounds, Cmp::Direction>().each() )
        {
            // flip and x-axis offset the sprite depending on the direction
            if( direction.x > 0 )
            {
                direction.x_scale = 1.f;
                direction.x_offset = 0.f;
            }
            else if ( direction.x < 0 )
            {
                direction.x_scale = -1.f;
                direction.x_offset = m_sprite_factory->DEFAULT_SPRITE_SIZE.x;
            }
            else 
            {
                direction.x_scale =  direction.x_scale; // keep last known direction
                direction.x_offset = direction.x_offset;
            }
            
            m_npc_ms->setScale({direction.x_scale, 1.f});
            m_npc_ms->setPosition({pos.x + direction.x_offset, pos.y});
            
            m_npc_ms->pick(2, "npc");
            m_window->draw(*m_npc_ms);
            
            // show npc scan distance
            if (m_show_path_distances)
            {
                sf::RectangleShape npc_square(npc_scan_bounds.size());
                npc_square.setFillColor(sf::Color::Transparent);
                npc_square.setOutlineColor(sf::Color::Red);
                npc_square.setOutlineThickness(1.f);
                npc_square.setPosition(npc_scan_bounds.position());
                m_window->draw(npc_square); 
            }
        }
    }

    void render_flood_waters()
    {
        for( auto [_, _wl]: m_flood_updates.view<Cmp::WaterLevel>().each() ) 
        {
            if( _wl.m_level > 0 )
            {
                m_water_shader.update(_wl.m_level);
            }
            m_window->draw(m_water_shader);
        }
    }

    void render_player_distances_on_npc()
    {
        if (!m_show_path_distances) return;

        // for (auto [entt, npc_position] : m_reg->view<Cmp::Position>().each())
        // {
        //     // sf::Text distance_text(m_font, "", 10);
        //     // if( player_distance_to_npc.distance == std::numeric_limits<unsigned int>::max() ) {
        //     //     continue;
        //     // } else {
        //     //     distance_text.setString(std::to_string(player_distance_to_npc.distance));
        //     // }

        //     // distance_text.setPosition(npc_position + sf::Vector2f{5.f, 0.f});
        //     // distance_text.setFillColor(sf::Color::White);
        //     // distance_text.setOutlineColor(sf::Color::Black);
        //     // distance_text.setOutlineThickness(2.f);
        //     // m_window->draw(distance_text);

        // }
    }

    void render_player_distances_on_obstacles()
    {
        if( !m_show_path_distances) return;
        auto obstacle_view = m_reg->view<Cmp::Obstacle, Cmp::Position, Cmp::PlayerDistance>();
        for (auto [_ob_entt, _ob, _ob_pos, _player_distance] : obstacle_view.each())
        {
            sf::Text distance_text(m_font, "", 10);
            distance_text.setString(std::to_string(_player_distance.distance));
            distance_text.setPosition(_ob_pos + sf::Vector2f{5.f, 0.f});
            distance_text.setFillColor(sf::Color::White);
            distance_text.setOutlineColor(sf::Color::Black);
            distance_text.setOutlineThickness(2.f);
            m_window->draw(distance_text);
        }
    }

    void render_npc_distances_on_obstacles()
    {
        if (!m_show_path_distances) return;

        auto entt_distance_map_view = m_reg->view<Cmp::EnttDistanceMap>();

        for( auto [npc_entt,distance_map] : entt_distance_map_view.each())
        {
            for (auto [obstacle_entt, distance] : distance_map)
            {
                auto obstacle_position = m_reg->try_get<Cmp::Position>(obstacle_entt);
                if( not obstacle_position ) continue;

                sf::Text distance_text(m_font, "", 10);

                distance_text.setString("+");

                distance_text.setPosition(*obstacle_position);
                distance_text.setFillColor(sf::Color::White);
                distance_text.setOutlineColor(sf::Color::Black);
                distance_text.setOutlineThickness(2.f);
                m_window->draw(distance_text);
            }
        }
    }

    ////////////////////////
    // MISC FUNCTIONS
    ////////////////////////


    bool isInView(const sf::View& view, const sf::Vector2f& position, const sf::Vector2f& size) {
        sf::FloatRect viewBounds(
            view.getCenter() - view.getSize() / 2.f,
            view.getSize()
        );
        sf::FloatRect objectBounds(position, size);
        
        return viewBounds.findIntersection(objectBounds) ? true : false;
    }

    void update_view_center(sf::View &view, Cmp::Position &player_pos)
    {
        const float MAP_HALF_WIDTH = view.getSize().x * 0.5f;
        const float MAP_HALF_HEIGHT = view.getSize().y * 0.5f;
        
        // Calculate the maximum allowed camera positions
        float maxX = DISPLAY_SIZE.x - MAP_HALF_WIDTH;
        float maxY = DISPLAY_SIZE.y - MAP_HALF_HEIGHT;
        
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

    ////// Overlay Functions

    void render_entt_distance_set_overlay(sf::Vector2f pos)
    {
        if (!m_show_path_distances) return;

        auto entt_distance_map_view = m_reg->view<Cmp::EnttDistanceMap>();
        int entt_distance_set = 0;
        for( auto [e,distance_map] : entt_distance_map_view.each())
        {

            sf::Text distance_text(m_font, "", 30);
            distance_text.setFillColor(sf::Color::White);
            distance_text.setOutlineColor(sf::Color::Black);
            distance_text.setOutlineThickness(2.f);

            if( distance_map.empty() ) {
                continue;
            } else {

                std::stringstream ss;
                ss << "NPC Entity #" << entt::to_integral(e) << " - ";
                distance_text.setPosition( pos + sf::Vector2f{0 , entt_distance_set * 30.f} );

                for(auto it = distance_map.begin(); it != distance_map.end(); ++it)
                {
                    ss  << " " 
                        << entt::to_integral(it->first)
                        << ":" << it->second
                        << ",";

                }

                distance_text.setString(ss.str());
                m_window->draw(distance_text);
            }
            entt_distance_set++;
        }
    }

    void render_bomb_radius_overlay(int radius_value, sf::Vector2f pos)
    {
        // text
        bomb_radius_text.setPosition(pos);
        bomb_radius_text.setFillColor(sf::Color::White);
        bomb_radius_text.setOutlineColor(sf::Color::Black);
        bomb_radius_text.setOutlineThickness(2.f);
        m_window->draw(bomb_radius_text);

        // text
        sf::Text bomb_radius_value_text(m_font, "", 30);
        bomb_radius_value_text.setString(std::to_string(radius_value));
        bomb_radius_value_text.setPosition(pos + sf::Vector2f(180.f, 0.f));
        bomb_radius_value_text.setFillColor(sf::Color::White);
        bomb_radius_value_text.setOutlineColor(sf::Color::Black);
        bomb_radius_value_text.setOutlineThickness(2.f);
        m_window->draw(bomb_radius_value_text);
    }

    void render_bomb_overlay(int bomb_count, sf::Vector2f pos)
    {
        // text
        bomb_inventory_text.setPosition(pos);
        bomb_inventory_text.setFillColor(sf::Color::White);
        bomb_inventory_text.setOutlineColor(sf::Color::Black);
        bomb_inventory_text.setOutlineThickness(2.f);
        m_window->draw(bomb_inventory_text);

        // text
        sf::Text bomb_count_text(m_font, "", 30);
        if (bomb_count < 0) bomb_count_text.setString(" INFINITE ");
        else bomb_count_text.setString(" x " + std::to_string(bomb_count));
        bomb_count_text.setPosition(pos + sf::Vector2f(100.f, 0.f));
        bomb_count_text.setFillColor(sf::Color::White);
        bomb_count_text.setOutlineColor(sf::Color::Black);
        bomb_count_text.setOutlineThickness(2.f);
        m_window->draw(bomb_count_text);
    }

    void render_health_overlay(float health_value, sf::Vector2f pos, sf::Vector2f size)
    {
        // text
        healthlvl_meter_text.setPosition(pos);
        healthlvl_meter_text.setFillColor(sf::Color::White);
        healthlvl_meter_text.setOutlineColor(sf::Color::Black);
        healthlvl_meter_text.setOutlineThickness(2.f);
        m_window->draw(healthlvl_meter_text);

        // bar fill
        sf::Vector2f healthbar_offset{100.f, 10.f};
        auto healthbar = sf::RectangleShape({((size.x / 100) * health_value), size.y});
        healthbar.setPosition(pos + healthbar_offset);
        healthbar.setFillColor(sf::Color::Red);
        m_window->draw(healthbar);

        // bar outline
        auto healthbar_border = sf::RectangleShape(size);
        healthbar_border.setPosition(pos + healthbar_offset);
        healthbar_border.setFillColor(sf::Color::Transparent);
        healthbar_border.setOutlineColor(sf::Color::Black);
        healthbar_border.setOutlineThickness(5.f);
        m_window->draw(healthbar_border);
    }

    void render_water_level_meter_overlay(float water_level, sf::Vector2f pos, sf::Vector2f size)
    {
        // text 
        waterlvl_meter_text.setPosition(pos);
        waterlvl_meter_text.setFillColor(sf::Color::White);
        waterlvl_meter_text.setOutlineColor(sf::Color::Black);
        waterlvl_meter_text.setOutlineThickness(2.f);
        m_window->draw(waterlvl_meter_text);

        // bar fill
        sf::Vector2f waterlvl_meter_offset{100.f, 10.f};
        // water meter level is represented as a percentage (0-100) of the screen display y-axis
        // note: {0,0} is top left so we need to invert the Y position
        float meter_meter_level = size.x - ((size.x / DISPLAY_SIZE.y) * water_level);
        auto waterlvlbar = sf::RectangleShape({ meter_meter_level, size.y});
        waterlvlbar.setPosition(pos + waterlvl_meter_offset);
        waterlvlbar.setFillColor(sf::Color::Blue);
        m_window->draw(waterlvlbar);

        // bar outline
        auto waterlvlbar_border = sf::RectangleShape(size);
        waterlvlbar_border.setPosition(pos + waterlvl_meter_offset);
        waterlvlbar_border.setFillColor(sf::Color::Transparent);
        waterlvlbar_border.setOutlineColor(sf::Color::Black);
        waterlvlbar_border.setOutlineThickness(5.f);
        m_window->draw(waterlvlbar_border);
    }



private:
    Sprites::FloodWaterShader m_water_shader{"res/FloodWater2.glsl", DISPLAY_SIZE};
    // SFML window handle
    std::shared_ptr<sf::RenderWindow> m_window;
    // path finding system handle
    std::shared_ptr<Sys::PathFindSystem> m_path_find_sys;
    
    // cached multi sprite objects created by SpriteFactory
    std::optional<Sprites::MultiSprite> m_rock_ms = m_sprite_factory->get_multisprite_by_type(Sprites::SpriteFactory::Type::ROCK);
    std::optional<Sprites::MultiSprite> m_pot_ms = m_sprite_factory->get_multisprite_by_type(Sprites::SpriteFactory::Type::POT);
    std::optional<Sprites::MultiSprite> m_bone_ms = m_sprite_factory->get_multisprite_by_type(Sprites::SpriteFactory::Type::BONES);
    std::optional<Sprites::MultiSprite> m_detonation_ms = m_sprite_factory->get_multisprite_by_type(Sprites::SpriteFactory::Type::DETONATED);
    std::optional<Sprites::MultiSprite> m_bomb_ms = m_sprite_factory->get_multisprite_by_type(Sprites::SpriteFactory::Type::BOMB);
    std::optional<Sprites::MultiSprite> m_wall_ms = m_sprite_factory->get_multisprite_by_type(Sprites::SpriteFactory::Type::WALL);
    std::optional<Sprites::MultiSprite> m_player_ms = m_sprite_factory->get_multisprite_by_type(Sprites::SpriteFactory::Type::PLAYER);
    std::optional<Sprites::MultiSprite> m_npc_ms = m_sprite_factory->get_multisprite_by_type(Sprites::SpriteFactory::Type::NPC);

    std::optional<Sprites::MultiSprite> m_extra_health_ms = m_sprite_factory->get_multisprite_by_type(Sprites::SpriteFactory::Type::EXTRA_HEALTH);
    std::optional<Sprites::MultiSprite> m_extra_bombs_ms = m_sprite_factory->get_multisprite_by_type(Sprites::SpriteFactory::Type::EXTRA_BOMBS);
    std::optional<Sprites::MultiSprite> m_infinite_bombs_ms = m_sprite_factory->get_multisprite_by_type(Sprites::SpriteFactory::Type::INFINI_BOMBS);
    std::optional<Sprites::MultiSprite> m_chain_bombs_ms = m_sprite_factory->get_multisprite_by_type(Sprites::SpriteFactory::Type::CHAIN_BOMBS);
    std::optional<Sprites::MultiSprite> m_lower_water_ms = m_sprite_factory->get_multisprite_by_type(Sprites::SpriteFactory::Type::LOWER_WATER);


    // background tile map
    Sprites::Containers::TileMap m_floormap;

    bool m_show_path_distances = false;
    bool m_show_armed_obstacles = false;

    Cmp::Font m_font = Cmp::Font("res/tuffy.ttf");
    sf::Text healthlvl_meter_text{m_font,   "Health:", 30};
    sf::Text waterlvl_meter_text{m_font,    "Flood:", 30};
    sf::Text bomb_inventory_text{m_font,    "Bombs:", 30};
    sf::Text bomb_radius_text{m_font,    "Blast Radius:", 30};

    const sf::Vector2f LOCAL_MAP_VIEW_SIZE{ 300.f, 200.f };
    const sf::Vector2f MINI_MAP_VIEW_SIZE{ DISPLAY_SIZE.x * 0.25f, DISPLAY_SIZE.y * 0.25f };

};

} // namespace ProceduralMaze::Systems

#endif // __SYSTEMS_RENDER_SYSTEM_HPP__