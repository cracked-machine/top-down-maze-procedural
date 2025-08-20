#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__

#include <EntityFactory.hpp>
#include <ProcGen/RandomLevelGenerator.hpp>
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
#include <Components/Movement.hpp>

#include <Systems/CollisionSystem.hpp>
#include <Systems/RenderSystem.hpp>
#include <Systems/ProcGen/CellAutomataSystem.hpp>

#include <EventHandler.hpp>
#include <Settings.hpp>

#ifdef _WIN32
    #include <windows.h>
#endif 

namespace ProceduralMaze {

class Engine {
public:
    Engine() {

        m_window->setFramerateLimit(144);

#ifdef _WIN32
        ::ShowWindow(m_window->getNativeHandle(), SW_MAXIMIZE);
#endif 

        register_reactive_storage();
    
        EntityFactory::add_system_entity( m_reg );
        EntityFactory::add_player_entity( m_reg );

        SPDLOG_INFO("Engine Init");

        Cmp::Random::seed(123456789); // for troubleshooting purposes

        // procedurally generate the level
        Sys::ProcGen::RandomLevelGenerator random_level(
            m_reg,
            Settings::WALL_TILE_POOL,
            Settings::BORDER_TILE_POOL
        );

        Sys::ProcGen::CellAutomataSystem cellauto_parser{random_level};
        cellauto_parser.iterate(m_reg, 5);
    }

    bool run()
    {
        sf::Clock deltaClock;

        /// MAIN LOOP BEGINS
        while (m_window->isOpen())
        {
            sf::Time deltaTime = deltaClock.restart();
            
            m_event_handler.handler(m_window, m_reg);
            process_movement(deltaTime);
            m_collsion_sys->track_path(m_reg);

            for(auto [_ent, _sys]: m_system_updates.view<Cmp::System>().each()) {
                if( _sys.collisions_enabled ) m_collsion_sys->check(m_reg);
            }
            
            m_render_sys->render(m_reg);   
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
    
    // move the player according to direction and delta time with acceleration
    void process_movement(sf::Time deltaTime)
    {
        const float dt = deltaTime.asSeconds();
        
        for(auto [_entt, _pc, _current_pos, _movement] : 
            m_reg.view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Movement>().each())
        {
            // Get input direction if available
            sf::Vector2f desired_direction(0.0f, 0.0f);
            if ( not m_event_handler.empty()) 
            {
                desired_direction = m_event_handler.pop();
            }

            // Apply acceleration in the desired direction
            if (desired_direction != sf::Vector2f(0.0f, 0.0f)) 
            {
                _movement.acceleration = desired_direction * _movement.acceleration_rate;
            } 
            else 
            {
                // Apply deceleration when no input
                if (_movement.velocity != sf::Vector2f(0.0f, 0.0f)) 
                {
                    _movement.acceleration = -_movement.velocity.normalized() * _movement.deceleration_rate;
                } 
                else 
                {
                    _movement.acceleration = sf::Vector2f(0.0f, 0.0f);
                }
            }

            // Update velocity
            _movement.velocity += _movement.acceleration * dt;

            // Get current velocity magnitude
            float speed = std::sqrt(_movement.velocity.x * _movement.velocity.x + 
                                  _movement.velocity.y * _movement.velocity.y);

            // Stop completely if current velocity magnitude is below minimum velocity
            if (_movement.velocity.length() < _movement.min_velocity) {
                _movement.velocity = sf::Vector2f(0.0f, 0.0f);
                _movement.acceleration = sf::Vector2f(0.0f, 0.0f);
            }
            // Clamp velocity to max speed if current velocity magnitude exceeds max speed
            else if (_movement.velocity.length() > _movement.max_speed) {
                _movement.velocity = (_movement.velocity / _movement.velocity.length()) * _movement.max_speed;
            }

            // Apply velocity to position
            _current_pos += _movement.velocity * dt;

            // if(_current_pos.y < Settings::MAP_GRID_OFFSET.y || 
            //    _current_pos.y > Settings::MAP_GRID_OFFSET.y + (Settings::MAP_GRID_SIZE.y * Settings::OBSTACLE_SIZE.y) ||
            //    _current_pos.x < 0 || 
            //    _current_pos.x > Settings::MAP_GRID_OFFSET.x + (Settings::MAP_GRID_SIZE.x * Settings::OBSTACLE_SIZE.x))
            // {
            //     // Reset position to starting point if player goes out of bounds
            //     _current_pos = Settings::PLAYER_START_POS;
            //     for (auto [_entt, _sys] : m_reg.view<Cmp::System>().each())
            //     {
            //         _sys.player_stuck = true;
            //     }
            //     return;
            // }
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