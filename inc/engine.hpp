#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics.hpp>

#include <random.hpp>
#include <random_coord.hpp>
#include <spdlog/spdlog.h>

#include <entt/entity/registry.hpp>

#include <components/position.hpp>
#include <components/font.hpp>
#include <systems/render_system.hpp>


namespace RENAME_THIS_NAMESPACE {

class Engine {
public:
    Engine() {
        
        m_window->setFramerateLimit(60);

        // independent of framerate
        m_render_period = sf::milliseconds(500);

        // register system listeners
        m_registry.on_update<Components::Position>().connect<&Systems::RenderSystem::update_cb>(m_render_sys);
    
        add_shape();
        add_text();

        SPDLOG_INFO("Engine Init");
    }

    bool run()
    {
        sf::Clock clock;
       
        bool init = true;

        while (m_window->isOpen())
        {
            while (const std::optional event = m_window->pollEvent())
            {
                if (event->is<sf::Event::Closed>()) { m_window->close(); }
            }
            
            // set the updates independent of the framerate
            if( ( clock.getElapsedTime() >=  m_render_period  ) || (init) )
            {
                m_window->clear();
                // get a view of all entitys that have a Positon and Random component
                for( auto [ _entt,  _pos, _randcoords] : 
                    m_registry.view<Components::Position, Components::RandomCoord>().each() )
                {
                    // set entity position to new random coords...
                    // RenderSystem is listening for updates, RenderSystem::update_cb() calls sfml::draw()
                    m_registry.patch<Components::Position>(_entt, [&](auto &_pos) { _pos = _randcoords.gen(); });
                    SPDLOG_DEBUG("Updating entity #{} position", entt::entt_traits<entt::entity>::to_entity(_entt));
                }
                m_window->display();
                clock.restart();
                init = false;
            }
        }
        return false;   
    }

private:
    // SFML Window
    std::shared_ptr<sf::RenderWindow> m_window = std::make_shared<sf::RenderWindow>(
        sf::VideoMode({1920u, 1080u}), "RENAME_THIS_SFML_WINDOW");
    
    // ECS Registry
    entt::basic_registry<entt::entity> m_registry;

    //  ECS Systems
    std::unique_ptr<Systems::RenderSystem> m_render_sys = 
        std::make_unique<Systems::RenderSystem> (m_window);

    sf::Time m_render_period;

    void add_text()
    {
        // add position component
        auto entity = m_registry.create();
        m_registry.emplace<Components::Position>(entity, sf::Vector2{0.f, 0.f}); 
        m_registry.emplace<Components::Font>(entity, "res/tuffy.ttf"); 
        m_registry.emplace<Components::RandomCoord>(entity, sf::Vector2f{m_window->getSize()});
    }
    void add_shape()
    {
        // add position component
        auto entity = m_registry.create();
        m_registry.emplace<Components::Position>(entity, sf::Vector2{0.f, 0.f}); 
        m_registry.emplace<Components::RandomCoord>(entity, sf::Vector2f{m_window->getSize()});

    }
};

} //namespace RENAME_THIS_NAMESPACE

#endif // __ENGINE_HPP__