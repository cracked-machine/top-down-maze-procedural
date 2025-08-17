#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics.hpp>


#include <brick.hpp>
#include <collision.hpp>
#include <collision_system.hpp>
#include <memory>

#include <components/obstacle.hpp>
#include <components/pc.hpp>

#include <procedural_generation/ca_system.hpp>
#include <procedural_generation/random_system.hpp>
#include <random.hpp>
#include <random_coord.hpp>
#include <spdlog/spdlog.h>
#include <event_handler.hpp>
#include <entt/entity/registry.hpp>

#include <components/position.hpp>
#include <components/font.hpp>
#include <systems/render_system.hpp>
#include <ybb.hpp>

const sf::Vector2u DISPLAY_SIZE{1920, 1024};
const sf::Vector2u MAP_GRID_SIZE{10u,10u};
const sf::Vector2f MAP_GRID_OFFSET{ 50.f,90.f};

namespace ProceduralMaze {

class Engine {
public:

    
    Engine() {
        m_window->setFramerateLimit(144);

        m_render_sys->m_position_updates.bind(m_reg);
        m_render_sys->m_position_updates
            .on_update<Cmp::Position>()
            .on_construct<Cmp::Position>();

        m_render_sys->m_position_updates.bind(m_reg);
        m_render_sys->m_position_updates
            .on_update<Cmp::Obstacle>()
            .on_construct<Cmp::Obstacle>();

        m_collsion_sys->m_position_updates.bind(m_reg);
        m_collsion_sys->m_position_updates
            .on_update<Cmp::Position>()
            .on_construct<Cmp::Position>();
    
        add_player( sf::Vector2f{10, 100} );
        
        // add a border
        for(int x = 0; x < DISPLAY_SIZE.x / Sprites::Brick::WIDTH; x++)
        {
            add_bedrock({
                x * (Sprites::Brick::WIDTH + (Sprites::Brick::LINEWIDTH * 2)), 
                MAP_GRID_OFFSET.y - (Sprites::Brick::HEIGHT + Sprites::Brick::LINEWIDTH + Sprites::Brick::LINEWIDTH)
            });
            add_bedrock({
                x * (Sprites::Brick::WIDTH + (Sprites::Brick::LINEWIDTH * 2)), 
                MAP_GRID_OFFSET.y + (MAP_GRID_SIZE.y * (Sprites::Brick::HEIGHT + Sprites::Brick::LINEWIDTH) )
            });
        }

        SPDLOG_INFO("Engine Init");
    }

    bool run()
    {
        Sys::ProcGen::RandomSystem randsys(MAP_GRID_SIZE, MAP_GRID_OFFSET);
        randsys.gen(m_reg, 123456789);
        Sys::ProcGen::CellAutomataSystem ca_level{randsys};

        int ca_count = 0;
        
        sf::Clock clock;
        
        while (m_window->isOpen())
        {
            auto elapsed_time = clock.getElapsedTime().asMilliseconds();
            if(elapsed_time > 10 and ca_count < 5)
            {
                ca_level.iterate_linear(m_reg);
                // ca_level.iterate_quadratic(m_reg);
                ca_count++;
                SPDLOG_INFO("Level Iteration #{} took {} ms", ca_count, elapsed_time);
                clock.restart();
            }
        
            m_event_handler.handler(m_window, m_reg);
            m_render_sys->render();         
            m_collsion_sys->check();
        
        }
        return false;   
    }

private:
    // SFML Window
    std::shared_ptr<sf::RenderWindow> m_window = std::make_shared<sf::RenderWindow>(
        sf::VideoMode(DISPLAY_SIZE), "ProceduralMaze");
    
    // ECS Registry
    entt::basic_registry<entt::entity> m_reg;

    //  ECS Systems
    std::unique_ptr<Sys::RenderSystem> m_render_sys = 
        std::make_unique<Sys::RenderSystem> (m_window);
    
    std::unique_ptr<Sys::CollisionSystem> m_collsion_sys =
        std::make_unique<Sys::CollisionSystem> ();

    ProceduralMaze::InputEventHandler m_event_handler;


    void add_text()
    {
        auto entity = m_reg.create();
        m_reg.emplace<Cmp::Position>(entity, sf::Vector2{0.f, 0.f}); 
        m_reg.emplace<Cmp::Font>(entity, "res/tuffy.ttf"); 
        m_reg.emplace<Cmp::RandomCoord>(entity, sf::Vector2f{m_window->getSize()});
    }

    void add_player(const sf::Vector2f &pos)
    {

        // To prevent possible sticky corners issue,
        // these should less than/equal to EventHandler::move_delta
        auto xbb_extra_width = 0.05f;
        auto ybb_extra_height = 0.05f;

        auto entity = m_reg.create();
        m_reg.emplace<Cmp::Position>(entity, pos); 
        m_reg.emplace<Cmp::PlayableCharacter>(entity );
        m_reg.emplace<Cmp::Xbb>(
            entity, 
            sf::Vector2f(pos.x - (xbb_extra_width / 2), pos.y), 
            sf::Vector2f(Sprites::Player::WIDTH + xbb_extra_width, Sprites::Player::HEIGHT) 
        );
        m_reg.emplace<Cmp::Ybb>(
            entity, 
            sf::Vector2f(pos.x, pos.y - (ybb_extra_height / 2)), 
            sf::Vector2f(Sprites::Player::WIDTH, Sprites::Player::HEIGHT + ybb_extra_height) 
        );
    }

    void add_bedrock(const sf::Vector2f &pos)
    {
        auto entity = m_reg.create();
        m_reg.emplace<Cmp::Position>(entity, pos); 
        m_reg.emplace<Cmp::Obstacle>(entity, Cmp::Obstacle::Type::BEDROCK, true, true );
    }
};

} //namespace ProceduralMaze

#endif // __ENGINE_HPP__