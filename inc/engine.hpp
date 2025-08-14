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

#include <random.hpp>
#include <random_coord.hpp>
#include <spdlog/spdlog.h>
#include <event_handler.hpp>
#include <entt/entity/registry.hpp>

#include <components/position.hpp>
#include <components/font.hpp>
#include <systems/render_system.hpp>


namespace ProceduralMaze {

class Engine {
public:
    Engine() {
        using namespace Systems;
        using namespace Components;
        m_window->setFramerateLimit(60);

        m_render_sys->m_position_updates.bind(m_reg);
        m_render_sys->m_position_updates
            .on_update<Position>()
            .on_construct<Position>();

        m_render_sys->m_collision_updates.bind(m_reg);
        m_render_sys->m_collision_updates
            .on_update<Collision>()
            .on_construct<Collision>();

        m_collsion_sys->m_position_updates.bind(m_reg);
        m_collsion_sys->m_position_updates
            .on_update<Position>()
            .on_construct<Position>();
    
        add_brick( sf::Vector2f{100, 100} );
        add_player( sf::Vector2f{10, 100} );

        SPDLOG_INFO("Engine Init");
    }

    bool run()
    {
        using namespace Components;
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
        sf::VideoMode({600u, 480u}), "RENAME_THIS_SFML_WINDOW");
    
    // ECS Registry
    entt::basic_registry<entt::entity> m_reg;

    //  ECS Systems
    std::unique_ptr<Systems::RenderSystem> m_render_sys = 
        std::make_unique<Systems::RenderSystem> (m_window);
    
    std::unique_ptr<Systems::CollisionSystem> m_collsion_sys =
        std::make_unique<Systems::CollisionSystem> ();
    

    ProceduralMaze::InputEventHandler m_event_handler;

    sf::Time m_render_period;

    void add_text()
    {
        using namespace Components;
        auto entity = m_reg.create();
        m_reg.emplace<Position>(entity, sf::Vector2{0.f, 0.f}); 
        m_reg.emplace<Font>(entity, "res/tuffy.ttf"); 
        m_reg.emplace<RandomCoord>(entity, sf::Vector2f{m_window->getSize()});
    }

    void add_player(const sf::Vector2f &pos)
    {
        using namespace Components;
        auto entity = m_reg.create();
        m_reg.emplace<Position>(entity, pos); 
        m_reg.emplace<PlayableCharacter>(entity );
    }

    void add_brick(const sf::Vector2f &pos)
    {
        using namespace Components;
        auto entity = m_reg.create();
        m_reg.emplace<Position>(entity, pos); 
        m_reg.emplace<Obstacle>(entity );
    }
};

} //namespace ProceduralMaze

#endif // __ENGINE_HPP__