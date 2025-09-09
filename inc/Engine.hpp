#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__

#include <BaseSystem.hpp>
#include <BombSystem.hpp>
#include <Direction.hpp>
#include <FloodSystem.hpp>
#include <GameState.hpp>
#include <NpcSystem.hpp>
#include <PathFindSystem.hpp>
#include <Persistent/FuseDelay.hpp>
#include <PlayerSystem.hpp>
#include <ProcGen/RandomLevelGenerator.hpp>
#include <RenderGameSystem.hpp>
#include <RenderMenuSystem.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <WaterLevel.hpp>
#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

#include <memory>

#include <Components/DisplaySize.hpp>
#include <Components/Font.hpp>
#include <Components/GameState.hpp>
#include <Components/Movement.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Position.hpp>
#include <Components/Random.hpp>
#include <Components/RandomCoord.hpp>
#include <Components/System.hpp>
#include <Sprites/TileMap.hpp>

#include <Systems/CollisionSystem.hpp>
#include <Systems/ProcGen/CellAutomataSystem.hpp>
#include <Systems/RenderSystem.hpp>

#include <EventHandler.hpp>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace ProceduralMaze {

class Engine
{
public:
  explicit Engine( std::shared_ptr<entt::basic_registry<entt::entity>> registry )
      : m_reg( std::move( registry ) ), m_sprite_factory( std::make_shared<Sprites::SpriteFactory>() ), m_player_sys( m_reg ), m_flood_sys( m_reg ),
        m_path_find_sys( m_reg ), m_npc_sys( m_reg ), m_collision_sys( m_reg ), m_render_game_sys( m_reg ), m_render_menu_sys( m_reg ),
        m_bomb_sys( m_reg ), m_event_handler( m_reg )
  {

    m_render_game_sys.window().setFramerateLimit( 144 );

#ifdef _WIN32
    ::ShowWindow( m_render_game_sys.window().getNativeHandle(), SW_MAXIMIZE );
#endif

    SPDLOG_INFO( "Engine Initiliasing: " );
    bootstrap();

    // Subscribe to NPC creation/death events
    std::ignore = Sys::BaseSystem::getEventDispatcher().sink<Events::NpcCreationEvent>().connect<&Sys::NpcSystem::on_npc_creation>( m_npc_sys );
    std::ignore = Sys::BaseSystem::getEventDispatcher().sink<Events::NpcDeathEvent>().connect<&Sys::NpcSystem::on_npc_death>( m_npc_sys );
    std::ignore = Sys::BaseSystem::getEventDispatcher().sink<Events::PlayerActionEvent>().connect<&Sys::BombSystem::on_player_action>( m_bomb_sys );

    // Cmp::Random::seed(123456789); // testing purposes
  }

  bool run()
  {
    sf::Clock deltaClock;

    /// MAIN LOOP BEGINS
    while ( m_render_game_sys.window().isOpen() )
    {
      sf::Time deltaTime = deltaClock.restart();

      auto gamestate_view = m_reg->view<Cmp::GameState>();
      for ( auto [entity, game_state] : gamestate_view.each() )
      {
        switch ( game_state.current_state )
        {
        case Cmp::GameState::State::MENU: {

          m_render_menu_sys.render_menu();
          m_event_handler.menu_state_handler( m_render_game_sys.window() );
          break;
        } // case MENU end

        case Cmp::GameState::State::SETTINGS: {
          m_render_menu_sys.render_settings( m_flood_sys, deltaTime );
          m_event_handler.settings_state_handler( m_render_game_sys.window() );
          break;
        } // case SETTINGS end

        case Cmp::GameState::State::LOADING: {
          setup();
          game_state.current_state = Cmp::GameState::State::PLAYING;
          SPDLOG_INFO( "Loading game...." );
          break;
        }

        case Cmp::GameState::State::UNLOADING: {
          teardown();
          bootstrap();
          game_state.current_state = Cmp::GameState::State::MENU;
          SPDLOG_INFO( "Unloading game...." );
          break;
        }

        case Cmp::GameState::State::PLAYING: {
          m_event_handler.game_state_handler( m_render_game_sys.window() );

          m_player_sys.update( deltaTime );
          m_flood_sys.update();
          m_bomb_sys.update();
          m_collision_sys.check_end_zone_collision();
          m_collision_sys.check_loot_collision();
          m_collision_sys.check_bones_reanimation();
          m_collision_sys.check_player_to_npc_collision();
          m_collision_sys.update_obstacle_distances();

          auto player_entity = m_reg->view<Cmp::PlayableCharacter>().front();
          for ( auto [_ent, _sys] : m_reg->view<Cmp::System>().each() )
          {
            if ( _sys.collisions_enabled ) m_collision_sys.check_collision();
            if ( _sys.level_complete )
            {
              SPDLOG_INFO( "Level complete!" );
              game_state.current_state = Cmp::GameState::State::GAMEOVER;
            }
          }

          m_path_find_sys.findPath( player_entity );
          m_npc_sys.lerp_movement( deltaTime );

          // did the player drown? Then end the game
          for ( auto [_, _pc] : m_reg->view<Cmp::PlayableCharacter>().each() )
          {
            if ( not _pc.alive ) { game_state.current_state = Cmp::GameState::State::GAMEOVER; }
          }

          m_render_game_sys.render_game();
          break;
        } // case PLAYING end

        case Cmp::GameState::State::PAUSED: {
          m_flood_sys.suspend();
          m_collision_sys.suspend();
          m_bomb_sys.suspend();

          while ( ( Cmp::GameState::State::PAUSED == game_state.current_state ) and m_render_game_sys.window().isOpen() )
          {
            m_render_menu_sys.render_paused();
            std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
            // check for keyboard/window events to keep window responsive
            m_event_handler.paused_state_handler( m_render_game_sys.window() );
          }

          m_flood_sys.resume();
          m_collision_sys.resume();
          m_bomb_sys.resume();

          break;
        } // case PAUSED end

        case Cmp::GameState::State::GAMEOVER: {
          for ( auto [_, _pc] : m_reg->view<Cmp::PlayableCharacter>().each() )
          {
            if ( not _pc.alive ) { m_render_menu_sys.render_defeat_screen(); }
            else { m_render_menu_sys.render_victory_screen(); }
          }
          std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
          m_event_handler.game_over_state_handler( m_render_game_sys.window() );

          break;
        } // case GAME_OVER end

        case Cmp::GameState::State::EXITING: {
          SPDLOG_INFO( "Terminating Game...." );

          teardown();
          m_render_game_sys.window().close();
          std::terminate();
        }
        }
      } // gamestate_view end

      // Update event dispatcher at end of frame
      Sys::BaseSystem::getEventDispatcher().update();

    } /// MAIN LOOP ENDS
    return false;
  }

private:
  // ECS registry
  std::shared_ptr<entt::basic_registry<entt::entity>> m_reg;

  // creates and manages MultiSprite resources
  std::shared_ptr<Sprites::SpriteFactory> m_sprite_factory;

  //  ECS Systems
  Sys::PlayerSystem m_player_sys;
  Sys::FloodSystem m_flood_sys;
  Sys::PathFindSystem m_path_find_sys;
  Sys::NpcSystem m_npc_sys;
  Sys::CollisionSystem m_collision_sys;
  Sys::RenderGameSystem m_render_game_sys;
  Sys::RenderMenuSystem m_render_menu_sys;
  Sys::BombSystem m_bomb_sys;

  // SFML keyboard/mouse event handler
  ProceduralMaze::InputEventHandler m_event_handler{ m_reg };

  // sets up ECS just enough to let the statemachine work
  void bootstrap()
  {
    SPDLOG_INFO( "bootstrap - start" );
    add_game_state_entity();
    SPDLOG_INFO( "bootstrap - game state entity added" );

    m_bomb_sys.init_context();
    m_player_sys.init_context();

    // we must have a sprite factory in the registry context
    // before it can be used by other systems that need it
    m_reg->ctx().emplace<std::shared_ptr<Sprites::SpriteFactory>>( m_sprite_factory );
    SPDLOG_INFO( "bootstrap - sprite factory added to context" );

    m_render_game_sys.load_multisprites();
    SPDLOG_INFO( "bootstrap - complete" );
  }

  // Sets up ECS for the rest of the game
  void setup()
  {
    reginfo( "Pre-setup" );

    // 2. setup new entities and generate the level
    add_system_entity();
    m_player_sys.add_player_entity();
    m_flood_sys.add_flood_water_entity();
    add_display_size( sf::Vector2u{ 1920, 1024 } );

    // create initial random game area with the required sprites
    std::unique_ptr<Sys::ProcGen::RandomLevelGenerator> random_level = std::make_unique<Sys::ProcGen::RandomLevelGenerator>( m_reg );

    // procedurally generate the game area from the initial random layout
    Sys::ProcGen::CellAutomataSystem cellauto_parser{ m_reg, std::move( random_level ) };
    cellauto_parser.iterate( 5 );

    reginfo( "Post-setup" );
  }

  // Teardown the engine
  void teardown()
  {
    SPDLOG_INFO( "Tearing down...." );
    reginfo( "Pre-teardown" );
    m_reg->clear();
    reginfo( "Post-teardown" );
  }

  void reginfo( std::string msg = "" )
  {
    std::size_t entity_count = 0;
    for ( [[maybe_unused]] auto entity : m_reg->view<entt::entity>() )
    {
      ++entity_count;
    }
    SPDLOG_INFO( "Registry Count - {}: {}", msg, entity_count );
  }

  void add_display_size( const sf::Vector2u &size )
  {
    if ( not m_reg->view<Cmp::DisplaySize>()->empty() )
    {
      SPDLOG_WARN( "Display size entity already exists, skipping creation" );
      return;
    }
    SPDLOG_INFO( "Creating display size entity" );
    auto entity = m_reg->create();
    m_reg->emplace<Cmp::DisplaySize>( entity, size );
  }

  void add_system_entity()
  {
    if ( not m_reg->view<Cmp::System>()->empty() )
    {
      SPDLOG_WARN( "System entity already exists, skipping creation" );
      return;
    }
    SPDLOG_INFO( "Creating system entity" );
    auto entity = m_reg->create();
    m_reg->emplace<Cmp::System>( entity );
  }

  void add_game_state_entity()
  {
    if ( not m_reg->view<Cmp::GameState>()->empty() )
    {
      SPDLOG_WARN( "Game state entity already exists, skipping creation" );
      return;
    }
    SPDLOG_INFO( "Creating game state entity" );
    auto entity = m_reg->create();
    m_reg->emplace<Cmp::GameState>( entity );
  }
};

} // namespace ProceduralMaze

#endif // __ENGINE_HPP__