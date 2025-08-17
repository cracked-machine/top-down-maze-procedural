#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics.hpp>

#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

#include <memory>

#include <Sprites/Brick.hpp>
#include <Sprites/TileMap.hpp>
#include <Components/Collision.hpp>
#include <Components/System.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Random.hpp>
#include <Components/RandomCoord.hpp>
#include <Components/Position.hpp>
#include <Components/Font.hpp>
#include <Components/System.hpp>
#include <Components/XAxisHitBox.hpp>
#include <Components/YAxisHitBox.hpp>

#include <Systems/CollisionSystem.hpp>
#include <Systems/RenderSystem.hpp>
#include <Systems/ProcGen/CellAutomataSystem.hpp>
#include <Systems/ProcGen/RandomSystem.hpp>

#include <EventHandler.hpp>
#include <Settings.hpp>

namespace ProceduralMaze {

class Engine {
public:
    Engine() {
        using namespace ProceduralMaze::Settings;
        m_window->setFramerateLimit(144);

        register_reactive_storage();
    
        add_system_entity();
        add_player_entity( PLAYER_START_POS );
        
        // add a border
        for(int x = 0; x < DISPLAY_SIZE.x / Sprites::Brick::WIDTH; x++)
        {
            // top edge
            add_bedrock_entity({
                x * (Sprites::Brick::WIDTH + (Sprites::Brick::LINEWIDTH * 2)), 
                MAP_GRID_OFFSET.y - (Sprites::Brick::HEIGHT + Sprites::Brick::LINEWIDTH + Sprites::Brick::LINEWIDTH)
            });
            // bottom edge
            add_bedrock_entity({
                x * (Sprites::Brick::WIDTH + (Sprites::Brick::LINEWIDTH * 2)), 
                MAP_GRID_OFFSET.y + (MAP_GRID_SIZE.y * (Sprites::Brick::HEIGHT + Sprites::Brick::LINEWIDTH) ) + Sprites::Brick::LINEWIDTH
            });
        }
        for( float y = 0; y < DISPLAY_SIZE.y; y += (Sprites::Brick::HEIGHT + (Sprites::Brick::LINEWIDTH*2)))
        {
            // left edge 
            add_bedrock_entity({0, y});
            add_bedrock_entity({static_cast<float>(DISPLAY_SIZE.x), y});

        }

        m_render_sys->m_current_view = sf::View(PLAYER_START_POS, {300.f, 200.f});

        SPDLOG_INFO("Engine Init");
    }

    bool run()
    {
        using namespace ProceduralMaze::Settings;

        // procedurally generate the level
        Sys::ProcGen::RandomSystem obstacle_randsys(MAP_GRID_SIZE, MAP_GRID_OFFSET);
        obstacle_randsys.gen(m_reg, 0);
        Sys::ProcGen::CellAutomataSystem ca_level{obstacle_randsys};

        sf::Clock clock;
        int level_gen_iteration_max = 5;
        for( int i = 0; i < level_gen_iteration_max; i++)
        {
            ca_level.iterate_linear(m_reg);
            SPDLOG_INFO("Level Iteration #{} took {} ms", i, clock.restart().asMilliseconds());
        }
        
        while (m_window->isOpen())
        {

            m_event_handler.handler(m_window, m_reg);
            m_collsion_sys->check();
            m_render_sys->render();   
        
        }
        return false;   
    }

private:
    // SFML Window
    std::shared_ptr<sf::RenderWindow> m_window = std::make_shared<sf::RenderWindow>(
        sf::VideoMode(ProceduralMaze::Settings::DISPLAY_SIZE), "ProceduralMaze");
    
    // ECS Registry
    entt::basic_registry<entt::entity> m_reg;

    //  ECS Systems
    std::unique_ptr<Sys::RenderSystem> m_render_sys = 
        std::make_unique<Sys::RenderSystem> (m_window);
    
    std::unique_ptr<Sys::CollisionSystem> m_collsion_sys =
        std::make_unique<Sys::CollisionSystem> ();

    ProceduralMaze::InputEventHandler m_event_handler;

    void add_system_entity()
    {
        auto entity = m_reg.create();
        m_reg.emplace<Cmp::System>(entity); 
    }

    void add_text_entity()
    {
        auto entity = m_reg.create();
        m_reg.emplace<Cmp::Position>(entity, sf::Vector2{0.f, 0.f}); 
        m_reg.emplace<Cmp::Font>(entity, "res/tuffy.ttf"); 
        m_reg.emplace<Cmp::RandomCoord>(entity, sf::Vector2f{m_window->getSize()});
    }

    void add_player_entity(const sf::Vector2f &pos)
    {
        auto entity = m_reg.create();
        m_reg.emplace<Cmp::Position>(entity, pos); 
        m_reg.emplace<Cmp::PlayableCharacter>(entity );
        m_reg.emplace<Cmp::Xbb>(
            entity, 
            sf::Vector2f(pos.x - (Cmp::Xbb::EXTRA_WIDTH / 2), pos.y), 
            sf::Vector2f(Sprites::Player::WIDTH + Cmp::Xbb::EXTRA_WIDTH, Sprites::Player::HEIGHT) 
        );
        m_reg.emplace<Cmp::Ybb>(
            entity, 
            sf::Vector2f(pos.x, pos.y - (Cmp::Ybb::EXTRA_HEIGHT / 2)), 
            sf::Vector2f(Sprites::Player::WIDTH, Sprites::Player::HEIGHT + Cmp::Ybb::EXTRA_HEIGHT) 
        );
    }

    void add_bedrock_entity(const sf::Vector2f &pos)
    {
        auto entity = m_reg.create();
        m_reg.emplace<Cmp::Position>(entity, pos); 
        m_reg.emplace<Cmp::Obstacle>(entity, Cmp::Obstacle::Type::BEDROCK, true, true );
    }

    void register_reactive_storage()
    {

        m_render_sys->m_system_updates.bind(m_reg);
        m_render_sys->m_system_updates
            .on_update<Cmp::System>()
            .on_construct<Cmp::System>();

        // render systems reactive storage for player position updates
        m_render_sys->m_position_updates.bind(m_reg);
        m_render_sys->m_position_updates
            .on_update<Cmp::Position>()
            .on_construct<Cmp::Position>();

        // render systems reactive storage for obstacle updates
        m_render_sys->m_position_updates.bind(m_reg);
        m_render_sys->m_position_updates
            .on_update<Cmp::Obstacle>()
            .on_construct<Cmp::Obstacle>();

        // Collision systems reactive storage for player position updates
        m_collsion_sys->m_position_updates.bind(m_reg);
        m_collsion_sys->m_position_updates
            .on_update<Cmp::Position>()
            .on_construct<Cmp::Position>();
    }

};

} //namespace ProceduralMaze

#endif // __ENGINE_HPP__