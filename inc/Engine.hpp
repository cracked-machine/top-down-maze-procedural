#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__

#include <BombSystem.hpp>
#include <Direction.hpp>
#include <EntityFactory.hpp>
#include <FloodSystem.hpp>
#include <GameState.hpp>
#include <PathFindSystem.hpp>
#include <ProcGen/RandomLevelGenerator.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics.hpp>

#include <WaterLevel.hpp>
#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

#include <memory>

#include <Sprites/TileMap.hpp>
#include <Components/System.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Random.hpp>
#include <Components/RandomCoord.hpp>
#include <Components/Position.hpp>
#include <Components/Font.hpp>
#include <Components/System.hpp>
#include <Components/Movement.hpp>

#include <Systems/CollisionSystem.hpp>
#include <Systems/RenderSystem.hpp>
#include <Systems/ProcGen/CellAutomataSystem.hpp>

#include <EventHandler.hpp>
#include <Settings.hpp>
#include <string>

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
        bootstrap();
        // Cmp::Random::seed(123456789); // testing purposes
    }

    bool run()
    {
        sf::Clock deltaClock;

        /// MAIN LOOP BEGINS
        while (m_window->isOpen())
        {
            sf::Time deltaTime = deltaClock.restart();

            auto gamestate_view = m_reg->view<Cmp::GameState>();
            for(auto [entity, game_state]: gamestate_view.each()) 
            {
                switch(game_state.current_state)
                {
                    case Cmp::GameState::State::MENU:
                    {
                        m_render_sys->render_menu();
                        m_event_handler.menu_state_handler(m_window);
                        break;
                    } // case MENU end

                    case Cmp::GameState::State::LOADING:
                    {
                        setup();
                        game_state.current_state = Cmp::GameState::State::PLAYING;
                        SPDLOG_INFO("Loading game....");
                        break;
                    }

                    case Cmp::GameState::State::UNLOADING:
                    {
                        teardown();
                        bootstrap();
                        game_state.current_state = Cmp::GameState::State::MENU;
                        SPDLOG_INFO("Unloading game....");
                        break;
                    }

                    case Cmp::GameState::State::PLAYING:
                    {
                        m_event_handler.game_state_handler(m_window);
        
                        update_character_movement(deltaTime);
                        process_action_queue();       
                        m_flood_sys->update();  
                        m_bomb_sys->update();
                        m_collision_sys->check_end_zone_collision();
                        m_collision_sys->check_loot_collision();
                        m_collision_sys->check_bones_reanimation();
                        m_collision_sys->check_npc_collision();

                        // did the player drown? Then end the game
                        for(auto [_, _pc]: m_reg->view<Cmp::PlayableCharacter>().each()) {
                            if ( not _pc.alive ) {
                                game_state.current_state = Cmp::GameState::State::GAMEOVER;
                            }
                        }

                        for(auto [_ent, _sys]: m_system_updates.view<Cmp::System>().each()) {
                            if( _sys.collisions_enabled ) m_collision_sys->check_collision();
                            if( _sys.level_complete )
                            {
                                SPDLOG_INFO("Level complete!");
                                game_state.current_state = Cmp::GameState::State::GAMEOVER;
                            }
                        }

                        auto player_entity = m_reg->view<Cmp::PlayableCharacter>().front();
                        m_path_find_sys->findPath(player_entity);
                        

                        m_render_sys->render_game();
                        break;
                    } // case PLAYING end

                    case Cmp::GameState::State::PAUSED:
                    {
                        m_flood_sys->suspend();
                        m_collision_sys->suspend();
                        m_bomb_sys->suspend();

                        // m_event_handler.paused_state_handler(m_window);

                        while( (Cmp::GameState::State::PAUSED == game_state.current_state) and m_window->isOpen())
                        {
                            m_render_sys->render_paused();
                            std::this_thread::sleep_for(std::chrono::milliseconds(200));
                            // check for keyboard/window events to keep window responsive
                            m_event_handler.paused_state_handler(m_window);
                        }

                        m_flood_sys->resume();
                        m_collision_sys->resume();
                        m_bomb_sys->resume();
           
                        break;
                    } // case PAUSED end

                    case Cmp::GameState::State::GAMEOVER:
                    {
                        for(auto [_, _pc]: m_reg->view<Cmp::PlayableCharacter>().each()) {
                            if ( not _pc.alive ) {      
                                m_render_sys->render_defeat_screen();
                            }
                            else {
                                m_render_sys->render_victory_screen();
                            }                  
                        }
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                        m_event_handler.game_over_state_handler(m_window);

                        break;
                    } // case GAME_OVER end

                    case Cmp::GameState::State::EXITING:
                    {
                        SPDLOG_INFO("Terminating Game....");
         
                        teardown();
                        m_window->close();
                        std::terminate();
                    }
     
                    }
            } // gamestate_view end
        } /// MAIN LOOP ENDS
        return false;   
    }

private:
    // SFML Window
    std::shared_ptr<sf::RenderWindow> m_window = std::make_shared<sf::RenderWindow>(
        sf::VideoMode(Settings::DISPLAY_SIZE), 
        "ProceduralMaze"
    );
    
    // ECS Registry
    std::shared_ptr<entt::basic_registry<entt::entity>> m_reg = 
        std::make_shared<entt::basic_registry<entt::entity>>(entt::basic_registry<entt::entity>{});

    //  ECS Systems
    std::shared_ptr<Sys::PathFindSystem> m_path_find_sys = std::make_shared<Sys::PathFindSystem>(m_reg);
    std::unique_ptr<Sys::CollisionSystem> m_collision_sys = std::make_unique<Sys::CollisionSystem>(m_reg);
    std::unique_ptr<Sys::RenderSystem> m_render_sys = std::make_unique<Sys::RenderSystem> (m_reg, m_window, m_path_find_sys);
    std::unique_ptr<Sys::FloodSystem> m_flood_sys = std::make_unique<Sys::FloodSystem> (m_reg, 5.f);
    std::unique_ptr<Sys::BombSystem> m_bomb_sys = std::make_unique<Sys::BombSystem> (
        m_reg, 
        m_render_sys->m_sprite_factory
    );

    // SFML keyboard/mouse event handler
    ProceduralMaze::InputEventHandler m_event_handler{m_reg};

    // pool for System component updates from the registry
    entt::reactive_mixin<entt::storage<void>> m_system_updates;
    entt::reactive_mixin<entt::storage<void>> m_gamestate_updates;



    void process_action_queue()
    {
        if (m_event_handler.m_action_queue.empty()) return;

        if (m_event_handler.m_action_queue.front() == InputEventHandler::GameActions::DROP_BOMB)
        {
            if( not m_event_handler.m_action_queue.empty() ) m_event_handler.m_action_queue.pop();
            m_bomb_sys->arm_occupied_location();
        }
    }

    // move the player according to direction and delta time with acceleration
    void update_character_movement(sf::Time deltaTime)
    {
        const float dt = deltaTime.asSeconds();

        for(auto [ entity, pc_cmp, pos_cmp, move_cmp, dir_cmp] : 
            m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Movement, Cmp::Direction>().each())
        {
            // Apply acceleration in the desired dir_cmp
            if (dir_cmp != sf::Vector2f(0.0f, 0.0f)) 
            {
                move_cmp.acceleration = dir_cmp * move_cmp.acceleration_rate;
            } 
            else 
            {
                // Apply deceleration when no input
                if (move_cmp.velocity != sf::Vector2f(0.0f, 0.0f)) 
                {
                    move_cmp.acceleration = -move_cmp.velocity.normalized() * move_cmp.deceleration_rate;
                } 
                else 
                {
                    move_cmp.acceleration = sf::Vector2f(0.0f, 0.0f);
                }
            }

            // Update velocity (change in velocity = acceleration * dt)
            move_cmp.velocity += move_cmp.acceleration * dt;

            // Stop completely if current velocity magnitude is below minimum velocity
            if (move_cmp.velocity.length() < move_cmp.min_velocity) {
                move_cmp.velocity = sf::Vector2f(0.0f, 0.0f);
                move_cmp.acceleration = sf::Vector2f(0.0f, 0.0f);
            }
            // Clamp velocity to max speed if current velocity magnitude exceeds max speed
            else if (move_cmp.velocity.length() > move_cmp.max_speed) {
                move_cmp.velocity = (move_cmp.velocity / move_cmp.velocity.length()) * move_cmp.max_speed;
            }

            // Apply velocity to position (change in position = velocity * dt)
            pos_cmp += move_cmp.velocity * dt;
    
        }
    }

    // sets up ECS just enough to let the statemachine work
    void bootstrap()
    {
        // Register this Engine's pool for GameState component updates
        m_gamestate_updates.bind(*m_reg);
        m_gamestate_updates
            .on_update<Cmp::GameState>()
            .on_construct<Cmp::GameState>();

        // Register the EventHandler's pool for GameState component updates
        m_event_handler.m_gamestate_updates.bind(*m_reg);
        m_event_handler.m_gamestate_updates
            .on_update<Cmp::GameState>()
            .on_construct<Cmp::GameState>();

        EntityFactory::add_game_state_entity( m_reg );
    }

    // Sets up ECS for the rest of the game
    void setup()
    {
        reginfo("Pre-setup");
        // 1. Register reactive storage containers to the registry

        // ENGINE
        //
        // Register this Engine's pool for System comnponent updates
        m_system_updates.bind(*m_reg);
        m_system_updates
            .on_update<Cmp::System>()
            .on_construct<Cmp::System>();

        // RENDERSYSTEM
        //
        // Register the RenderSystem's pool for System comnponent updates
        m_render_sys->m_system_updates.bind(*m_reg);
        m_render_sys->m_system_updates
            .on_update<Cmp::System>()
            .on_construct<Cmp::System>();

        // Register the RenderSystem's pool for Position comnponent updates
        // basically every entity...
        m_render_sys->m_position_updates.bind(*m_reg);
        m_render_sys->m_position_updates
            .on_update<Cmp::Position>()
            .on_construct<Cmp::Position>();

        m_render_sys->m_flood_updates.bind(*m_reg);
        m_render_sys->m_flood_updates
            .on_update<Cmp::WaterLevel>()
            .on_construct<Cmp::WaterLevel>();

        // Register the RenderSystem's pool for Obstacle comnponent updates
        m_render_sys->m_position_updates.bind(*m_reg);
        m_render_sys->m_position_updates
            .on_update<Cmp::Obstacle>()
            .on_construct<Cmp::Obstacle>();

        // COLLISIONSYSTEM
        //
        // Register the CollisionSystem's pool for Position comnponent updates
        // basically every entity...
        m_collision_sys->m_collision_updates.bind(*m_reg);
        m_collision_sys->m_collision_updates
            .on_update<Cmp::Position>()
            .on_construct<Cmp::Position>();

        // 2. setup new entities and generate the level
        EntityFactory::add_system_entity( m_reg );
        EntityFactory::add_player_entity( m_reg );
        EntityFactory::add_entt_distance_priority_queue( m_reg );
        m_flood_sys->add_flood_water_entity();

        // create initial random game area with the required sprites
        std::unique_ptr<Sys::ProcGen::RandomLevelGenerator> random_level = std::make_unique<Sys::ProcGen::RandomLevelGenerator>(
            m_reg,
            m_render_sys->m_sprite_factory
        );

        // procedurally generate the game area from the initial random layout
        Sys::ProcGen::CellAutomataSystem cellauto_parser{m_reg, std::move(random_level)};
        cellauto_parser.iterate(5);

        m_render_sys->create_debug_id_texture();

        reginfo("Post-setup");
        queueinfo();
    }

    // Teardown the engine and clear all event queues.
    // It clears all reactive storage, resets their connection to the registry 
    // and finally clears the registry.
    // If you need to restart the game, you should call bootstrap() immediately
    // after calling this function.
    void teardown()
    {
        SPDLOG_INFO("Tearing down....");
        reginfo("Pre-teardown");

        m_event_handler.m_action_queue = {};    

        m_system_updates.clear();
        m_system_updates.reset();
        m_gamestate_updates.clear();
        m_gamestate_updates.reset();
        m_event_handler.m_gamestate_updates.clear();
        m_event_handler.m_gamestate_updates.reset();
        m_render_sys->m_system_updates.clear();
        m_render_sys->m_system_updates.reset();
        m_render_sys->m_position_updates.clear();
        m_render_sys->m_position_updates.reset();
        m_render_sys->m_flood_updates.clear();
        m_render_sys->m_flood_updates.reset();
        m_collision_sys->m_collision_updates.clear();
        m_collision_sys->m_collision_updates.reset();

        m_reg->clear();
        reginfo("Post-teardown");
        queueinfo();
    }

    void reginfo(std::string msg = "")
    {
        std::size_t entity_count = 0;
        for([[maybe_unused]] auto entity: m_reg->view<entt::entity>()) { ++entity_count; }
        SPDLOG_INFO("Registry Count - {}: {}", msg, entity_count);
    }

    void queueinfo()
    {
        SPDLOG_INFO("{} action events pending", m_event_handler.m_action_queue.size()); 
    }

};

} //namespace ProceduralMaze

#endif // __ENGINE_HPP__