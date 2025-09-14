#include <Engine.hpp>
#include <Persistent/MusicVolume.hpp>
#include <Persistent/ObstaclePushBack.hpp>

namespace ProceduralMaze {

Engine::Engine( std::shared_ptr<entt::basic_registry<entt::entity>> registry )
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
  // m_title_music.setLooping( true );

  init_context();
  m_bomb_sys.init_context();
  m_player_sys.init_context();
  m_flood_sys.init_context();
  m_collision_sys.init_context();
  m_npc_sys.init_context();
  m_path_find_sys.init_context();
}

bool Engine::run()
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

        if ( m_title_music.getStatus() != sf::Music::Status::Playing ) { m_title_music.play(); }
        m_title_music.setVolume( m_reg->ctx().get<Cmp::Persistent::MusicVolume>()() );

        m_render_menu_sys.render_title();
        m_event_handler.menu_state_handler( m_render_game_sys.window() );
        break;
      } // case MENU end

      case Cmp::GameState::State::SETTINGS: {
        m_render_menu_sys.render_settings( deltaTime );
        m_event_handler.settings_state_handler( m_render_game_sys.window() );

        // make volume changes immediately audible
        m_title_music.setVolume( m_reg->ctx().get<Cmp::Persistent::MusicVolume>()() );
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
        if ( m_title_music.getStatus() == sf::Music::Status::Playing ) { m_title_music.stop(); }
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
          if ( _sys.collisions_enabled ) m_collision_sys.check_player_obstacle_collision();
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

        m_render_game_sys.render_game( deltaTime );
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
        if ( m_title_music.getStatus() == sf::Music::Status::Playing ) { m_title_music.stop(); }
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

// sets up ECS just enough to let the statemachine work
void Engine::bootstrap()
{
  SPDLOG_INFO( "bootstrap - start" );
  add_game_state_entity();
  SPDLOG_INFO( "bootstrap - game state entity added" );

  // we must have a sprite factory in the registry context
  // before it can be used by other systems that need it
  m_reg->ctx().emplace<std::shared_ptr<Sprites::SpriteFactory>>( m_sprite_factory );
  SPDLOG_INFO( "bootstrap - sprite factory added to context" );

  m_render_game_sys.load_multisprites();
  SPDLOG_INFO( "bootstrap - complete" );
}

void Engine::init_context()
{
  if ( not m_reg->ctx().contains<Cmp::Persistent::MusicVolume>() ) { m_reg->ctx().emplace<Cmp::Persistent::MusicVolume>(); }
}

// Sets up ECS for the rest of the game
void Engine::setup()
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

  // Reset the views early to prevent wild panning back to the start
  // position when the game starts rendering
  m_render_game_sys.reset_views();

  reginfo( "Post-setup" );
}

// Teardown the engine
void Engine::teardown()
{
  SPDLOG_INFO( "Tearing down...." );
  reginfo( "Pre-teardown" );
  m_reg->clear();
  reginfo( "Post-teardown" );
}

void Engine::reginfo( std::string msg )
{
  std::size_t entity_count = 0;
  for ( [[maybe_unused]] auto entity : m_reg->view<entt::entity>() )
  {
    ++entity_count;
  }
  SPDLOG_INFO( "Registry Count - {}: {}", msg, entity_count );
}

void Engine::add_display_size( const sf::Vector2u &size )
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

void Engine::add_system_entity()
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

void Engine::add_game_state_entity()
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

} // namespace ProceduralMaze