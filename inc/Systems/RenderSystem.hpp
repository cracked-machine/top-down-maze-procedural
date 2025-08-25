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


        m_multi_sprites = m_sprite_factory->create_multisprites_list();

        rocksprite = m_multi_sprites[static_cast<uint32_t>(SpriteFactory::Type::ROCK)];
        potsprite = m_multi_sprites[static_cast<uint32_t>(SpriteFactory::Type::POT)];
        bonesprite = m_multi_sprites[static_cast<uint32_t>(SpriteFactory::Type::BONES)];
        playersprite = m_multi_sprites[static_cast<uint32_t>(SpriteFactory::Type::PLAYER)];
        bombsprite = m_multi_sprites[static_cast<uint32_t>(SpriteFactory::Type::BOMB)];
        detonationsprite = m_multi_sprites[static_cast<uint32_t>(SpriteFactory::Type::DETONATED)];
        wallsprite = m_multi_sprites[static_cast<uint32_t>(SpriteFactory::Type::WALL)];


        SPDLOG_DEBUG("RenderSystem()"); 
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
                render_floormap({0, Settings::MAP_GRID_OFFSET.y * Sprites::Brick::HEIGHT});
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
                render_floormap({0, Settings::MAP_GRID_OFFSET.y * Sprites::Brick::HEIGHT});
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

    void render_obstacles()
    {

        for( auto [entity, _ob, _pos, _ob_nb_list]: 
            m_position_updates.view<Cmp::Obstacle, Cmp::Position, Cmp::Neighbours>().each() ) {
            
            // debug mode
            if( m_show_obstacle_debug )
            {
                if( _ob.m_enabled ) 
                {
                    sf::RectangleShape temp_square(Settings::OBSTACLE_SIZE_2F);
                    temp_square.setPosition(_pos);
                    temp_square.setFillColor(sf::Color::Transparent);
                    temp_square.setOutlineColor(sf::Color::White);
                    temp_square.setOutlineThickness(1.f);
                    m_window->draw(temp_square);
                }
            }
            else 
            {
                // ROCK/POT/BONES objects
                if( _ob.m_enabled ) 
                {
                    switch(_ob.m_type) 
                    {
                        case SpriteFactory::Type::ROCK:
                            rocksprite.pick(_ob.m_tile_index, "Obstacle");
                            rocksprite.setPosition(_pos);
                            m_window->draw(rocksprite);
                            break;
                        case SpriteFactory::Type::POT:
                            potsprite.pick(_ob.m_tile_index, "Obstacle");
                            potsprite.setPosition(_pos);
                            m_window->draw(potsprite);
                            break;
                        case SpriteFactory::Type::BONES:
                            bonesprite.pick(_ob.m_tile_index, "Obstacle");
                            bonesprite.setPosition(_pos);
                            m_window->draw(bonesprite);
                            break;
                        default:
                            break;
                    }

                }
                // "empty" sprite for detonated objects
                if( _ob.m_broken) 
                {
                    detonationsprite.setPosition(_pos);
                    detonationsprite.pick(0, "Detonated");
                    m_window->draw(detonationsprite);
                }
            }
        }

        // render armed obstacles with debug outlines
        for( auto [entity, _ob, _armed, _pos, _ob_nb_list]: 
            m_position_updates.view<Cmp::Obstacle, Cmp::Armed, Cmp::Position, Cmp::Neighbours>().each() ) {

            // Draw a red square around the obstacle we are standing on
            sf::RectangleShape temp_square(Settings::OBSTACLE_SIZE_2F);
            temp_square.setPosition(_pos);
            temp_square.setFillColor(sf::Color::Transparent);
            temp_square.setOutlineColor(sf::Color::Red);
            temp_square.setOutlineThickness(1.f);
            m_window->draw(temp_square);

            bombsprite.setPosition(_pos);
            bombsprite.pick(0, "Bomb");
            m_window->draw(bombsprite);

            // get each neighbour entity from the current obstacles neighbour list
            // and draw a blue square around it
            for( auto [_dir, _nb_entt] : _ob_nb_list) 
            {
                sf::RectangleShape nb_square(Settings::OBSTACLE_SIZE_2F);

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
                wallsprite.pick(0, "wall");
                wallsprite.setPosition(_pos);
                m_window->draw(wallsprite);
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
            playersprite.setPosition({_pos.x, _pos.y - (Settings::PLAYER_SPRITE_SIZE.y / 2.f)});
            playersprite.pick(1, "player");
            m_window->draw(playersprite);
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
    std::shared_ptr<SpriteFactory> m_sprite_factory = std::make_shared<SpriteFactory>();

    private:
    
    // Entity registry
    std::shared_ptr<entt::basic_registry<entt::entity>> m_reg;
    
    // list of multi sprite objects returned by SpriteFactory
    std::vector<Sprites::MultiSprite> m_multi_sprites;
    Sprites::MultiSprite rocksprite;
    Sprites::MultiSprite potsprite;
    Sprites::MultiSprite bonesprite;
    Sprites::MultiSprite detonationsprite;
    Sprites::MultiSprite bombsprite;
    Sprites::MultiSprite playersprite;
    Sprites::MultiSprite wallsprite;



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