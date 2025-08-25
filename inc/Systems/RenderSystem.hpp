#ifndef __SYSTEMS_RENDER_SYSTEM_HPP__
#define __SYSTEMS_RENDER_SYSTEM_HPP__

#include <Armed.hpp>
#include <BasicSprite.hpp>
#include <DebugEntityIds.hpp>
#include <FloodWater.hpp>
#include <MultiSprite.hpp>
#include <Neighbours.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

#include <Font.hpp>
#include <SFML/System/Vector2.hpp>
#include <SpriteFactory.hpp>
#include <WaterLevel.hpp>
#include <cstddef>
#include <entt/entity/fwd.hpp>
#include <exception>
#include <memory>

#include <Obstacle.hpp>
#include <PlayableCharacter.hpp>
#include <Position.hpp>
#include <Settings.hpp>
#include <System.hpp>
#include <Systems/BaseSystem.hpp>
#include <spdlog/spdlog.h>
#include <TileMap.hpp>

namespace ProceduralMaze::Sys {

class RenderSystem  {
public:
    RenderSystem(
        std::shared_ptr<entt::basic_registry<entt::entity>> reg,
        std::shared_ptr<sf::RenderWindow> win
    ) : 
        m_reg( reg ),
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
        
        // init local view dimensions
        m_local_view = sf::View( 
            { Settings::LOCAL_MAP_VIEW_SIZE.x * 0.5f, Settings::DISPLAY_SIZE.y * 0.5f}, 
            Settings::LOCAL_MAP_VIEW_SIZE 
        );
        m_local_view.setViewport( sf::FloatRect({0.f, 0.f}, {1.f, 1.f}) );

        // init minimap view dimensions
        m_minimap_view = sf::View( 
            { Settings::MINI_MAP_VIEW_SIZE.x * 0.5f, Settings::DISPLAY_SIZE.y * 0.5f}, 
            Settings::MINI_MAP_VIEW_SIZE 
        );
        m_minimap_view.setViewport( sf::FloatRect({0.75f, 0.f}, {0.25f, 0.25f}) );

        // we should ensure these member variables are initialized
        if( not m_rock_ms ) { SPDLOG_CRITICAL("Unable to get ROCK multisprite from SpriteFactory"); std::get_terminate(); }
        if( not m_pot_ms ) { SPDLOG_CRITICAL("Unable to get POT multisprite from SpriteFactory"); std::get_terminate(); }
        if( not m_bone_ms ) { SPDLOG_CRITICAL("Unable to get BONE multisprite from SpriteFactory"); std::get_terminate(); }
        if( not m_player_ms ) { SPDLOG_CRITICAL("Unable to get PLAYER multisprite from SpriteFactory"); std::get_terminate(); }
        if( not m_bomb_ms ) { SPDLOG_CRITICAL("Unable to get BOMB multisprite from SpriteFactory"); std::get_terminate(); }
        if( not m_detonation_ms ) { SPDLOG_CRITICAL("Unable to get DETONATION multisprite from SpriteFactory"); std::get_terminate(); }
        if( not m_wall_ms ) { SPDLOG_CRITICAL("Unable to get WALL multisprite from SpriteFactory"); std::get_terminate(); }

        SPDLOG_INFO("RenderSystem initialised..."); 
    }
    
    ~RenderSystem() { SPDLOG_DEBUG("~RenderSystem()"); } 

    void render_menu()
    {
        // main render begin
        m_window->clear();
        {
            sf::Text title_text(m_font, "Procedural Maze Game", 96);
            title_text.setFillColor(sf::Color::White);
            title_text.setPosition({Settings::DISPLAY_SIZE.x / 4.f, 100.f});
            m_window->draw(title_text);

            sf::Text start_text(m_font, "Press <Enter> key to start", 48);
            start_text.setFillColor(sf::Color::White);
            start_text.setPosition({Settings::DISPLAY_SIZE.x / 4.f, 200.f});
            m_window->draw(start_text);
        }
        m_window->display();
        // main render end
    }

    void render_deathscreen()
    {
        // main render begin
        m_window->clear();
        {
            sf::Text title_text(m_font, "You died!", 96);
            title_text.setFillColor(sf::Color::White);
            title_text.setPosition({Settings::DISPLAY_SIZE.x / 4.f, 100.f});
            m_window->draw(title_text);

            sf::Text start_text(m_font, "Press any key to continue", 48);
            start_text.setFillColor(sf::Color::White);
            start_text.setPosition({Settings::DISPLAY_SIZE.x / 4.f, 200.f});
            m_window->draw(start_text);
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
            title_text.setPosition({Settings::DISPLAY_SIZE.x / 4.f, 100.f});
            m_window->draw(title_text);

            sf::Text start_text(m_font, "Press P to continue", 48);
            start_text.setFillColor(sf::Color::White);
            start_text.setPosition({Settings::DISPLAY_SIZE.x / 4.f, 200.f});
            m_window->draw(start_text);
        }
        m_window->display();
        // main render end
    }

    void render_game()
    {
        using namespace Sprites;

        for(auto [_ent, _sys]: m_system_updates.view<Cmp::System>().each()) {
            m_show_obstacle_debug = _sys.show_obstacle_entity_id;
        }                  

        // main render begin
        m_window->clear();
        {
            // local view begin - this shows only a `Settings::LOCAL_MAP_VIEW_SIZE` of the game world
            m_window->setView(m_local_view);
            {   
                render_floormap({0, Settings::MAP_GRID_OFFSET.y * m_sprite_factory->DEFAULT_SPRITE_SIZE.y});
                render_obstacles();
                render_player();
                render_flood_waters();

                // move the local view position to equal the player position
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

                // show debug entity id text
                if( m_show_obstacle_debug ) {
                    auto s = m_debug_mode_entity_text.getSprite();
                    // s.setPosition({10, 10}); // Fixed position in screen coordinates
                    m_window->draw(s);
                }                

            } 
            // local view end

            // minimap view begin - this show a quarter of the game world but in a much smaller scale
            m_window->setView(m_minimap_view);
            {
                render_floormap({0, Settings::MAP_GRID_OFFSET.y * m_sprite_factory->DEFAULT_SPRITE_SIZE.y});
                render_obstacles();
                render_player();
                render_flood_waters();

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

            // UI Overlays begin (these will always be displayed no matter where the player moves)
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

                for(auto [_entt, _pc]: m_reg->view<Cmp::PlayableCharacter>().each()) {
                    render_health_overlay(_pc.health, {20.f, 20.f},  {200.f, 20.f});
                }

            } 
            // UI Overlays end

        } 
        m_window->display();
        // main render end
    }

    void render_health_overlay(float health_value, sf::Vector2f pos, sf::Vector2f size)
    {
        auto healthbar = sf::RectangleShape({((size.x / 100) * health_value), size.y});
        healthbar.setPosition(pos);
        healthbar.setFillColor(sf::Color::Red);
        m_window->draw(healthbar);
        auto healthbar_border = sf::RectangleShape(size);
        healthbar_border.setPosition(pos);
        healthbar_border.setFillColor(sf::Color::Transparent);
        healthbar_border.setOutlineColor(sf::Color::Black);
        healthbar_border.setOutlineThickness(5.f);
        m_window->draw(healthbar_border);
    }


    void render_floormap(const sf::Vector2f &offset = {0.f, 0.f})
    {
        m_floormap.setPosition(offset);
        m_window->draw(m_floormap);
    }

    bool isInView(const sf::View& view, const sf::Vector2f& position, const sf::Vector2f& size) {
        sf::FloatRect viewBounds(
            view.getCenter() - view.getSize() / 2.f,
            view.getSize()
        );
        sf::FloatRect objectBounds(position, size);
        
        return viewBounds.findIntersection(objectBounds) ? true : false;
    }

    void render_obstacles() {

        // Group similar draw operations to reduce state changes
        std::vector<std::pair<sf::Vector2f, int>> rockPositions;
        std::vector<std::pair<sf::Vector2f, int>> potPositions;
        std::vector<std::pair<sf::Vector2f, int>> bonePositions;
        std::vector<sf::Vector2f> wallPositions;
        std::vector<sf::Vector2f> detonationPositions;
        
        // Collect all positions first instead of drawing immediately
        for(auto [entity, _ob, _pos, _ob_nb_list]: 
            m_position_updates.view<Cmp::Obstacle, Cmp::Position, Cmp::Neighbours>().each()) {
            
            if(m_show_obstacle_debug) {
                if(_ob.m_enabled) {
                    // Store debug squares for batch rendering
                    // ...
                }
            } else {
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
                if(_ob.m_broken) {
                    detonationPositions.push_back(_pos);
                }
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

        // render armed obstacles with debug outlines
        for( auto [entity, _ob, _armed, _pos, _ob_nb_list]: 
            m_position_updates.view<Cmp::Obstacle, Cmp::Armed, Cmp::Position, Cmp::Neighbours>().each() ) {

            // Draw a red square around the obstacle we are standing on
            sf::RectangleShape temp_square(sf::Vector2f{m_sprite_factory->DEFAULT_SPRITE_SIZE});
            temp_square.setPosition(_pos);
            temp_square.setFillColor(sf::Color::Transparent);
            temp_square.setOutlineColor(sf::Color::Red);
            temp_square.setOutlineThickness(1.f);
            m_window->draw(temp_square);

            m_bomb_ms->setPosition(_pos);
            m_bomb_ms->pick(0, "Bomb");
            m_window->draw(*m_bomb_ms);

            // get each neighbour entity from the current obstacles neighbour list
            // and draw a blue square around it
            for( auto [_dir, _nb_entt] : _ob_nb_list) 
            {
                sf::RectangleShape nb_square(sf::Vector2f{m_sprite_factory->DEFAULT_SPRITE_SIZE});

                Cmp::Position* _nb_entt_pos = m_reg->try_get<Cmp::Position>( entt::entity(_nb_entt) );

                if( not _nb_entt_pos )
                {
                    SPDLOG_WARN("Unable to find Position component for entity: {}", entt::to_integral(_nb_entt));
                    assert(_nb_entt_pos && "Unable to find Position component for entity" && entt::to_integral(_nb_entt));
                    continue;
                }

                nb_square.setPosition(*_nb_entt_pos);                  
                nb_square.setFillColor(sf::Color::Transparent);
                nb_square.setOutlineColor(sf::Color::Blue); 
                nb_square.setOutlineThickness(1.f); 
                m_window->draw(nb_square);
            }
                    
        }

        // Render textures for "WALL" entities - filtered out because they don't own neighbour components
        for( auto [entity, _ob, _pos]: 
            m_position_updates.view<Cmp::Obstacle, Cmp::Position>(entt::exclude<Cmp::Neighbours>).each() ) {

            if( _ob.m_enabled) 
            {
                m_wall_ms->pick(0, "wall");
                m_wall_ms->setPosition(_pos);
                m_window->draw(*m_wall_ms);
            }            
        }
    }

    void render_flood_waters()
    {
        for( auto [_, _wl]: m_flood_updates.view<Cmp::WaterLevel>().each() ) 
        {
            m_window->draw(Sprites::FloodWaters{
                sf::Vector2f{Settings::DISPLAY_SIZE},
                {0, _wl.m_level}
            });
        }
    }

    void render_player()
    {
        for( auto [entity, _pc, _pos]: 
            m_position_updates.view<Cmp::PlayableCharacter, Cmp::Position>().each() ) 
        {
            m_player_ms->setPosition({_pos.x, _pos.y - (m_sprite_factory->PLAYER_SPRITE_SIZE.y / 2.f)});
            m_player_ms->pick(1, "player");
            m_window->draw(*m_player_ms);
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

    void create_debug_id_texture()
    {
        for( auto [_entt, _ob, _pos]: 
            m_position_updates.view<Cmp::Obstacle, Cmp::Position>().each() )
        {
            m_debug_mode_entity_text.addEntity(
                _ob.m_tile_index,
                sf::Vector2f{_pos.x, _pos.y}
            );
        }
    }

    entt::reactive_mixin<entt::storage<void>> m_position_updates;
    entt::reactive_mixin<entt::storage<void>> m_system_updates;
    entt::reactive_mixin<entt::storage<void>> m_flood_updates;

    sf::View m_local_view;
    sf::View m_minimap_view;

    // creates and manages MultiSprite resources
    std::shared_ptr<Sprites::SpriteFactory> m_sprite_factory = std::make_shared<Sprites::SpriteFactory>();

private:
    
    // Entity registry
    std::shared_ptr<entt::basic_registry<entt::entity>> m_reg;
    
    // cached multi sprite objects created by SpriteFactory
    std::optional<Sprites::MultiSprite> m_rock_ms = m_sprite_factory->get_multisprite_by_type(Sprites::SpriteFactory::Type::ROCK);
    std::optional<Sprites::MultiSprite> m_pot_ms = m_sprite_factory->get_multisprite_by_type(Sprites::SpriteFactory::Type::POT);
    std::optional<Sprites::MultiSprite> m_bone_ms = m_sprite_factory->get_multisprite_by_type(Sprites::SpriteFactory::Type::BONES);
    std::optional<Sprites::MultiSprite> m_detonation_ms = m_sprite_factory->get_multisprite_by_type(Sprites::SpriteFactory::Type::DETONATED);
    std::optional<Sprites::MultiSprite> m_bomb_ms = m_sprite_factory->get_multisprite_by_type(Sprites::SpriteFactory::Type::BOMB);
    std::optional<Sprites::MultiSprite> m_wall_ms = m_sprite_factory->get_multisprite_by_type(Sprites::SpriteFactory::Type::WALL);
    std::optional<Sprites::MultiSprite> m_player_ms = m_sprite_factory->get_multisprite_by_type(Sprites::SpriteFactory::Type::PLAYER);

    // SFML window handle
    std::shared_ptr<sf::RenderWindow> m_window;

    // background tile map
    Sprites::Containers::TileMap m_floormap;

    Sprites::Containers::DebugEntityIds m_debug_mode_entity_text{m_font};
    bool m_show_obstacle_debug = false;

    Cmp::Font m_font = Cmp::Font("res/tuffy.ttf");
// 
};

} // namespace ProceduralMaze::Systems

#endif // __SYSTEMS_RENDER_SYSTEM_HPP__