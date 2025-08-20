#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__

#include <EntityFactory.hpp>
#include <ProcGen/RandomObstacleGenerator.hpp>
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

#include <EventHandler.hpp>
#include <Settings.hpp>

namespace ProceduralMaze {

class Engine {
public:
    Engine() {

        m_window->setFramerateLimit(144);

        register_reactive_storage();
    
        EntityFactory::add_system_entity( m_reg );
        EntityFactory::add_player_entity( m_reg );

        SPDLOG_INFO("Engine Init");

        // procedurally generate the level
        Sys::ProcGen::RandomTilePlacer tile_placer(
            m_reg,
            Settings::WALL_TILE_POOL,
            Settings::BORDER_TILE_POOL
        );

        Sys::ProcGen::CellAutomataSystem cellauto_parser{tile_placer};
        for( int i = 0; i < 5; i++)
        {
            SPDLOG_INFO(
                "Level Iteration #{} took {} ms", 
                i, 
                cellauto_parser.iterate_linear(m_reg).asMilliseconds()
            );
        }
    }

    bool run()
    {
        /// MAIN LOOP BEGINS
        sf::Clock deltaClock;
        while (m_window->isOpen())
        {
            sf::Time deltaTime = deltaClock.restart();
            
            m_event_handler.handler(m_window, m_reg);
            process_movement(deltaTime);

            for(auto [_ent, _sys]: m_system_updates.view<Cmp::System>().each()) {
                if( _sys.collisions_enabled ) m_collsion_sys->check(m_reg);
            }
            
            m_render_sys->render();   
        }
        /// MAIN LOOP ENDS
        return false;   
    }

private:
    // SFML Window
    std::shared_ptr<sf::RenderWindow> m_window = std::make_shared<sf::RenderWindow>(
        sf::VideoMode(Settings::DISPLAY_SIZE), 
        "ProceduralMaze"
    );
    
    // ECS Registry
    entt::basic_registry<entt::entity> m_reg;

    //  ECS Systems
    std::unique_ptr<Sys::RenderSystem> m_render_sys = std::make_unique<Sys::RenderSystem> (m_window);
    std::unique_ptr<Sys::CollisionSystem> m_collsion_sys = std::make_unique<Sys::CollisionSystem>();

    // SFML keyboard/mouse event handler
    ProceduralMaze::InputEventHandler m_event_handler;

    // pool for System component updates from the registry
    entt::reactive_mixin<entt::storage<void>> m_system_updates;
    
    // move the player according to direction and delta time
    void process_movement(sf::Time deltaTime)
    {
        if( m_event_handler.empty() ) { return; }

        const float movement_speed = 100.0f; // pixels per second
        auto new_direction = m_event_handler.next();
        auto scaled_movement = new_direction * movement_speed * deltaTime.asSeconds();
        
        for( auto [ _entt, _pc, _current_pos] : 
            m_reg.view<Cmp::PlayableCharacter, Cmp::Position>().each() )
        {
            _current_pos += scaled_movement;
        }
    }

    // Register the systems to use their reactive storage
    // This allows the systems to react to changes in the registry
    // such as when a component is added or updated
    void register_reactive_storage()
    {
        // Register this Engine's pool for System comnponent updates
        m_system_updates.bind(m_reg);
        m_system_updates
            .on_update<Cmp::System>()
            .on_construct<Cmp::System>();

        // Register the RenderSystem's pool for System comnponent updates
        m_render_sys->m_system_updates.bind(m_reg);
        m_render_sys->m_system_updates
            .on_update<Cmp::System>()
            .on_construct<Cmp::System>();

        // Register the RenderSystem's pool for Position comnponent updates
        m_render_sys->m_position_updates.bind(m_reg);
        m_render_sys->m_position_updates
            .on_update<Cmp::Position>()
            .on_construct<Cmp::Position>();

        // Register the RenderSystem's pool for Obstacle comnponent updates
        m_render_sys->m_position_updates.bind(m_reg);
        m_render_sys->m_position_updates
            .on_update<Cmp::Obstacle>()
            .on_construct<Cmp::Obstacle>();

        // Register the CollisionSystem's pool for Position comnponent updates
        m_collsion_sys->m_position_updates.bind(m_reg);
        m_collsion_sys->m_position_updates
            .on_update<Cmp::Position>()
            .on_construct<Cmp::Position>();
    }

};

} //namespace ProceduralMaze

#endif // __ENGINE_HPP__