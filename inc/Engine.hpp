#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__

#include <EntityFactory.hpp>
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
    
        EntityFactory::add_system_entity( m_reg );
        EntityFactory::add_player_entity( m_reg, PLAYER_START_POS );
        EntityFactory::add_border( m_reg );

        m_render_sys->m_local_view = sf::View(PLAYER_START_POS, {300.f, 200.f});

        SPDLOG_INFO("Engine Init");
    }

    bool run()
    {
        using namespace ProceduralMaze::Settings;

        // procedurally generate the level
        Sys::ProcGen::RandomSystem obstacle_randsys;
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
            process_movement();

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
    
    void process_movement()
    {
        if( m_event_handler.empty() ) { return; }

        auto new_direction = m_event_handler.next();
        for( auto [ _entt, _pc, _current_pos, _xbb, _ybb] : 
            m_reg.view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Xbb, Cmp::Ybb>().each() )
        {
            _current_pos += new_direction;
            _xbb.position += new_direction;
            _ybb.position += new_direction;
        }
        
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