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

        
    
        SPDLOG_INFO("Engine Initiliasing: ");
        SPDLOG_INFO("{} system events pending",  m_event_handler.m_system_action_queue.size());
        SPDLOG_INFO("{} direction events pending", m_event_handler.m_direction_queue.size());
        SPDLOG_INFO("{} action events pending", m_event_handler.m_action_queue.size());

        // Cmp::Random::seed(123456789); // for troubleshooting purposes
    }

    bool run()
    {
        sf::Clock deltaClock;

        /// MAIN LOOP BEGINS
        while (m_window->isOpen())
        {
            sf::Time deltaTime = deltaClock.restart();
            m_event_handler.handler(m_window, m_reg, m_game_state);

            switch(m_game_state)
            {
         
            case Settings::GameState::MENU:
                          
                if (m_event_handler.m_system_action_queue.front() == InputEventHandler::SystemActions::START_GAME)
                {
                    SPDLOG_INFO("Entering game....");
                    m_event_handler.m_system_action_queue.pop();
                    m_game_state = Settings::GameState::PLAYING;

                    setup();
                }

                m_render_sys->render_menu();
                break;

            case Settings::GameState::PLAYING:
                
                if (m_event_handler.m_system_action_queue.front() == InputEventHandler::SystemActions::PAUSE_GAME)
                {
                    m_event_handler.m_system_action_queue.pop();
                    m_game_state = Settings::GameState::PAUSED;
                }
                if (m_event_handler.m_system_action_queue.front() == InputEventHandler::SystemActions::QUIT_GAME)
                {
                    m_event_handler.m_system_action_queue.pop();
                    m_game_state = Settings::GameState::MENU;
                    teardown();
                    queueinfo();
                    
                    SPDLOG_INFO("Entering menu....");
                    break;
                }

                process_direction_queue(deltaTime);
                process_action_queue();            
            
                for(auto [_ent, _sys]: m_system_updates.view<Cmp::System>().each()) {
                    if( _sys.collisions_enabled ) m_collision_sys->check(m_reg);
                }              

                m_render_sys->render_game(m_reg);     
                break;

            case Settings::GameState::PAUSED:
                SPDLOG_INFO("Game Paused....");
                m_render_sys->render_paused();
                if (m_event_handler.m_system_action_queue.front() == InputEventHandler::SystemActions::RESUME_GAME)
                {
                    m_event_handler.m_system_action_queue.pop();
                    m_game_state = Settings::GameState::PLAYING;
                    SPDLOG_INFO("Resuming Game....");
                }
                break;
            case Settings::GameState::GAME_OVER:
                m_render_sys->render_deathscreen();

                if (m_event_handler.m_system_action_queue.front() == InputEventHandler::SystemActions::QUIT_GAME)
                {
                    SPDLOG_INFO("Game Over....");
                    m_event_handler.m_system_action_queue.pop();
                    m_game_state = Settings::GameState::MENU;
                    teardown();
                    queueinfo();
                    
                    SPDLOG_INFO("Entering menu....");
                    break;
                }
                break;
            }
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
    std::unique_ptr<Sys::CollisionSystem> m_collision_sys = std::make_unique<Sys::CollisionSystem>();

    // SFML keyboard/mouse event handler
    ProceduralMaze::InputEventHandler m_event_handler;

    // pool for System component updates from the registry
    entt::reactive_mixin<entt::storage<void>> m_system_updates;

    Settings::GameState m_game_state = Settings::GameState::MENU;

    // Register reactive storage containers to the registry
    void setup()
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
        // basically every entity...
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
        // basically every entity...
        m_collision_sys->m_collision_updates.bind(m_reg);
        m_collision_sys->m_collision_updates
            .on_update<Cmp::Position>()
            .on_construct<Cmp::Position>();
    }

    // Teardown the engine and clear all event queues
    // This is called when the game is over or the engine is shutting down
    // It clears all reactive storage, resets their connection to the registry 
    // and finally clears the registry
    void teardown()
    {
        SPDLOG_INFO("Tearing down....");
        m_event_handler.m_direction_queue = {};
        m_event_handler.m_action_queue = {};
        m_event_handler.m_system_action_queue = {};     

        m_system_updates.clear();
        m_system_updates.reset();
        m_render_sys->m_system_updates.clear();
        m_render_sys->m_system_updates.reset();
        m_render_sys->m_position_updates.clear();
        m_render_sys->m_position_updates.reset();
        m_collision_sys->m_collision_updates.clear();
        m_collision_sys->m_collision_updates.reset();

        m_reg.clear();

    }

    void queueinfo()
    {
        SPDLOG_INFO("{} system events pending",  m_event_handler.m_system_action_queue.size());
        SPDLOG_INFO("{} direction events pending", m_event_handler.m_direction_queue.size());
        SPDLOG_INFO("{} action events pending", m_event_handler.m_action_queue.size()); 
    }

    void process_action_queue()
    {
        bool place_bomb = false;
        if( m_event_handler.m_action_queue.front() == InputEventHandler::GameActions::DROP_BOMB )
        {
            m_event_handler.m_action_queue.pop();
            place_bomb = true;
        }
        m_collision_sys->track_path(m_reg, place_bomb);    
    }

    // move the player according to direction and delta time with acceleration
    void process_direction_queue(sf::Time deltaTime)
    {
        const float dt = deltaTime.asSeconds();
        
        for(auto [_entt, _pc, _current_pos, _movement] : 
            m_reg.view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Movement>().each())
        {
            // Get input direction if available
            sf::Vector2f desired_direction(0.0f, 0.0f);
            if ( not m_event_handler.m_direction_queue.empty()) 
            {
                desired_direction = m_event_handler.m_direction_queue.front();
                m_event_handler.m_direction_queue.pop();
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

};

} //namespace ProceduralMaze

#endif // __ENGINE_HPP__