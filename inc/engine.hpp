#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics.hpp>


#include <collision.hpp>
#include <collision_system.hpp>
#include <memory>

#include <components/obstacle.hpp>
#include <components/pc.hpp>

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


namespace ProceduralMaze {

class Engine {
public:
    Engine() {
        m_window->setFramerateLimit(60);

        m_render_sys->m_position_updates.bind(m_reg);
        m_render_sys->m_position_updates
            .on_update<Cmp::Position>()
            .on_construct<Cmp::Position>();

        m_render_sys->m_collision_updates.bind(m_reg);
        m_render_sys->m_collision_updates
            .on_update<Cmp::Collision>()
            .on_construct<Cmp::Collision>();

        m_collsion_sys->m_position_updates.bind(m_reg);
        m_collsion_sys->m_position_updates
            .on_update<Cmp::Position>()
            .on_construct<Cmp::Position>();
    
        add_player( sf::Vector2f{10, 100} );
        Sys::ProcGen::RandomSystem random_level{
            sf::Vector2u(10,10), 
            m_reg,
            sf::Vector2f{50, 50}
        };
        // add_brick( sf::Vector2f{100, 100} );
        // add_brick( sf::Vector2f{120, 100} );
        // add_brick( sf::Vector2f{140, 100} );
        // add_brick( sf::Vector2f{100, 120} );
        // add_brick( sf::Vector2f{100, 140} );


        SPDLOG_INFO("Engine Init");
    }

    bool run()
    {
        sf::Clock clock;
        
        while (m_window->isOpen())
        {
            m_event_handler.handler(m_window, m_reg);
            m_render_sys->render();         
            m_collsion_sys->check();
        }
        return false;   
    }

private:
    // SFML Window
    std::shared_ptr<sf::RenderWindow> m_window = std::make_shared<sf::RenderWindow>(
        sf::VideoMode({550u, 550u}), "ProceduralMaze");
    
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
        auto xbb_extra_width = 1.f;
        auto ybb_extra_height = 1.f;

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

    void add_brick(const sf::Vector2f &pos)
    {
        auto entity = m_reg.create();
        m_reg.emplace<Cmp::Position>(entity, pos); 
        m_reg.emplace<Cmp::Obstacle>(entity );
    }
};

} //namespace ProceduralMaze

#endif // __ENGINE_HPP__