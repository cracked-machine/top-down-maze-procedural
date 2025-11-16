#include <Components/CorruptionCell.hpp>
#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/Persistent/MusicVolume.hpp>
#include <Components/PlayerMortality.hpp>
#include <Components/SinkholeCell.hpp>
#include <Engine.hpp>
#include <Events/PauseClocksEvent.hpp>
#include <Events/ResumeClocksEvent.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/Threats/HazardFieldSystem.hpp>

#include <memory>
#include <stacktrace>
#include <thread>

namespace ProceduralMaze
{

Engine::Engine()
{

  if ( !m_window->isOpen() )
  {
    SPDLOG_CRITICAL( "Failed to create SFML RenderWindow" );
    std::terminate();
  }

  m_window->clear( sf::Color::Black );
  m_window->display();

  // setup ImGui here rather than RenderSystem classes to reduce white screen init time
  if ( not ImGui::SFML::Init( *m_window ) )
  {
    SPDLOG_CRITICAL( "ImGui-SFML initialization failed" );
    std::terminate();
  }

  m_window->setVerticalSyncEnabled( true );
  // m_render_game_sys.window().setFramerateLimit( 144 );

  // Set ImGui style
  ImGuiIO &io = ImGui::GetIO();
  io.FontGlobalScale = 1.5f;
  io.IniFilename = "res/imgui.ini";
  std::ignore = ImGui::SFML::UpdateFontTexture();
}

bool Engine::run()
{
  try
  {
    loading_screen( [this]() { this->init_systems(); }, m_splash_texture );

    sf::Clock globalFrameClock;

    /// MAIN LOOP BEGINS
    while ( m_window->isOpen() )
    {
      sf::Time globalDeltaTime = globalFrameClock.restart();
      auto &game_state = m_event_handler->get_persistent_component<Cmp::Persistent::GameState>();

      switch ( game_state.current_state )
      {

        case Cmp::Persistent::GameState::State::MENU:
        {
          m_render_menu_sys->render_title();

          // update fx volumes with persistent settings
          auto &effects_volume = m_event_handler->get_persistent_component<Cmp::Persistent::EffectsVolume>().get_value();
          m_sound_bank->update_effects_volume( effects_volume );

          // update music volumes with persistent settings
          auto &music_volume = m_event_handler->get_persistent_component<Cmp::Persistent::MusicVolume>().get_value();
          m_sound_bank->update_music_volume( music_volume );

          if ( m_sound_bank->get_music( "title_music" ).getStatus() != sf::Music::Status::Playing )
          {
            m_sound_bank->get_music( "title_music" ).play();
          }

          m_event_handler->menu_state_handler();
          break;
        } // case MENU end

        case Cmp::Persistent::GameState::State::SETTINGS:
        {

          m_render_menu_sys->render_settings( globalDeltaTime );
          m_event_handler->settings_state_handler();
          break;
        } // case SETTINGS end

        case Cmp::Persistent::GameState::State::LOADING:
        {
          // wait for fade out to complete

          loading_screen( [this]() { this->enter_game(); }, m_splash_texture );

          game_state.current_state = Cmp::Persistent::GameState::State::PLAYING;
          SPDLOG_INFO( "Loading game...." );
          break;
        }

        case Cmp::Persistent::GameState::State::UNLOADING:
        {
          m_sound_bank->get_music( "game_music" ).stop();

          m_player_sys->stop_footsteps_sound();
          exit_game();
          game_state.current_state = Cmp::Persistent::GameState::State::MENU;
          SPDLOG_INFO( "Unloading game...." );
          break;
        }

        case Cmp::Persistent::GameState::State::PLAYING:
        {
          m_sound_bank->get_music( "title_music" ).stop();
          if ( m_sound_bank->get_music( "game_music" ).getStatus() != sf::Music::Status::Playing )
          {
            m_sound_bank->get_music( "game_music" ).play();
          }

          // play/stop footstep sounds depending on player movement
          auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Direction>();
          for ( auto [pc_entity, pc_cmp, dir_cmp] : player_view.each() )
          {
            if ( dir_cmp == sf::Vector2f( 0.f, 0.f ) ) { m_player_sys->stop_footsteps_sound(); }
            else { m_player_sys->play_footsteps_sound(); }
          }

          m_event_handler->game_state_handler();

          // m_flood_sys.update();
          m_anim_sys->update( globalDeltaTime );
          m_sinkhole_sys->update_hazard_field();
          m_corruption_sys->update_hazard_field();
          m_bomb_sys->update();

          m_sinkhole_sys->check_npc_hazard_field_collision();
          m_corruption_sys->check_npc_hazard_field_collision();
          m_exit_sys->check_exit_collision();
          m_loot_sys->check_loot_collision();
          m_npc_sys->check_bones_reanimation();
          m_wormhole_sys->check_player_wormhole_collision();
          m_digging_sys->update();
          m_footstep_sys->update();

          // Throttled obstacle distance update (optimization)
          if ( m_obstacle_distance_timer.getElapsedTime() >= m_obstacle_distance_update_interval )
          {
            m_path_find_sys->update_player_distances();
            m_obstacle_distance_timer.restart();
          }

          // enable/disable collision detection depending on Cmp::System settings
          for ( auto [_ent, _sys] : m_reg->view<Cmp::System>().each() )
          {
            m_player_sys->update_movement( globalDeltaTime, !_sys.collisions_enabled );
            if ( _sys.collisions_enabled )
            {
              m_sinkhole_sys->check_player_hazard_field_collision();
              m_corruption_sys->check_player_hazard_field_collision();
              m_npc_sys->check_player_to_npc_collision();
            }
            if ( _sys.level_complete )
            {
              SPDLOG_INFO( "Level complete!" );
              game_state.current_state = Cmp::Persistent::GameState::State::GAMEOVER;
            }
          }

          auto player_entity = m_reg->view<Cmp::PlayableCharacter>().front();
          m_path_find_sys->findPath( player_entity );
          m_npc_sys->update_movement( globalDeltaTime );

          // did the player die? Then end the game
          if ( m_player_sys->check_player_mortality() == Cmp::PlayerMortality::State::DEAD )
          {
            SPDLOG_INFO( "Player has died!" );
            game_state.current_state = Cmp::Persistent::GameState::State::GAMEOVER;
          }

          m_render_game_sys->render_game( globalDeltaTime, *m_render_overlay_sys, *m_render_player_sys );
          break;
        } // case PLAYING end

        case Cmp::Persistent::GameState::State::PAUSED:
        {
          m_event_handler->getEventDispatcher().trigger( Events::PauseClocksEvent() );
          globalFrameClock.stop();

          while ( ( Cmp::Persistent::GameState::State::PAUSED == game_state.current_state ) and m_window->isOpen() )
          {
            m_render_menu_sys->render_paused( globalDeltaTime );
            std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
            // check for keyboard/window events to keep window responsive
            m_event_handler->paused_state_handler();
          }
          // save persistent settings and update music/sfx volumes with persistent settings
          m_event_handler->getEventDispatcher().trigger( Events::SaveSettingsEvent() );
          auto &effects_volume = m_event_handler->get_persistent_component<Cmp::Persistent::EffectsVolume>().get_value();
          m_sound_bank->update_effects_volume( effects_volume );
          auto &music_volume = m_event_handler->get_persistent_component<Cmp::Persistent::MusicVolume>().get_value();
          m_sound_bank->update_music_volume( music_volume );

          m_event_handler->getEventDispatcher().trigger( Events::ResumeClocksEvent() );
          globalFrameClock.start();

          break;
        } // case PAUSED end

        case Cmp::Persistent::GameState::State::GAMEOVER:
        {
          m_player_sys->stop_footsteps_sound();

          if ( m_player_sys->check_player_mortality() == Cmp::PlayerMortality::State::DEAD )
          {
            m_render_menu_sys->render_defeat_screen();
          }
          else { m_render_menu_sys->render_victory_screen(); }

          std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
          m_event_handler->game_over_state_handler();

          break;
        } // case GAME_OVER end

        case Cmp::Persistent::GameState::State::EXITING:
        {
          SPDLOG_INFO( "Terminating application...." );
          exit_game();
          m_window->close();
          break;
        }
      }

      // Update event dispatcher at end of frame
      Sys::BaseSystem::getEventDispatcher().update();

    } /// MAIN LOOP ENDS
  }
  catch ( const std::exception &e )
  {
    SPDLOG_CRITICAL( "Unhandled exception in Engine::run(): {}", e.what() );
    show_error_screen( e.what() );
    return false;
  }
  catch ( ... )
  {
    SPDLOG_CRITICAL( "Unhandled unknown exception in Engine::run()" );
    show_error_screen( "An unknown error has occurred. Please check log." );
    return false;
  }
  return true; // exit game
}

void Engine::init_systems()
{

  // init core systems - these are required just to get the engine running
  m_sprite_factory = std::make_unique<Sprites::SpriteFactory>();
  m_sprite_factory->init();
  m_render_menu_sys = std::make_unique<Sys::RenderMenuSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_render_menu_sys->setRegistry( m_reg.get() );
  m_event_handler = std::make_unique<Sys::EventHandler>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_event_handler->setRegistry( m_reg.get() );
  m_event_handler->add_persistent_component<Cmp::Persistent::GameState>();
  m_persistent_sys = std::make_unique<Sys::PersistentSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_persistent_sys->setRegistry( m_reg.get() );
  m_persistent_sys->initializeComponentRegistry();
  m_persistent_sys->load_state();

  m_sound_bank->init();

  // init game systems - these are required for the actual gameplay
  // might as well init them all here....it will shorten load times later
  m_render_game_sys = std::make_unique<Sys::RenderGameSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_render_game_sys->setRegistry( m_reg.get() );

  m_player_sys = std::make_unique<Sys::PlayerSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_player_sys->setRegistry( m_reg.get() );

  m_path_find_sys = std::make_unique<Sys::PathFindSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_path_find_sys->setRegistry( m_reg.get() );

  m_npc_sys = std::make_unique<Sys::NpcSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_npc_sys->setRegistry( m_reg.get() );

  m_collision_sys = std::make_unique<Sys::CollisionSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_collision_sys->setRegistry( m_reg.get() );

  m_digging_sys = std::make_unique<Sys::DiggingSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_digging_sys->setRegistry( m_reg.get() );

  m_render_overlay_sys = std::make_unique<Sys::RenderOverlaySystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_render_overlay_sys->setRegistry( m_reg.get() );

  m_render_player_sys = std::make_unique<Sys::RenderPlayerSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_render_player_sys->setRegistry( m_reg.get() );

  m_bomb_sys = std::make_unique<Sys::BombSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_bomb_sys->setRegistry( m_reg.get() );

  m_loot_sys = std::make_unique<Sys::LootSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_loot_sys->setRegistry( m_reg.get() );

  m_anim_sys = std::make_unique<Sys::AnimSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_anim_sys->setRegistry( m_reg.get() );

  m_sinkhole_sys = std::make_unique<Sys::SinkHoleHazardSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_sinkhole_sys->setRegistry( m_reg.get() );
  m_sinkhole_sys->init();

  m_corruption_sys = std::make_unique<Sys::CorruptionHazardSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_corruption_sys->setRegistry( m_reg.get() );
  m_corruption_sys->init();

  m_wormhole_sys = std::make_unique<Sys::WormholeSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_wormhole_sys->setRegistry( m_reg.get() );
  m_wormhole_sys->init();

  m_exit_sys = std::make_unique<Sys::ExitSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_exit_sys->setRegistry( m_reg.get() );

  m_footstep_sys = std::make_unique<Sys::FootstepSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_footstep_sys->setRegistry( m_reg.get() );

  m_large_obstacle_sys = std::make_unique<Sys::LargeObstacleSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_large_obstacle_sys->setRegistry( m_reg.get() );

  m_render_game_sys->init_shaders();
  m_render_game_sys->init_tilemap();

  add_display_size( sf::Vector2u{ 1920, 1024 } );

  // prep the title screen resources since they get used next
  m_render_menu_sys->init_title();
  SPDLOG_INFO( "Lazy initialization of systems complete" );
}

// Sets up ECS for the rest of the game
void Engine::enter_game()
{
  add_system_entity();
  m_player_sys->add_player_entity();

  // create initial random game area with the required sprites
  std::unique_ptr<Sys::ProcGen::RandomLevelGenerator> random_level = std::make_unique<Sys::ProcGen::RandomLevelGenerator>(
      *m_window, *m_sprite_factory, *m_sound_bank );
  random_level->setRegistry( m_reg.get() );
  random_level->generate();

  // procedurally generate the game area from the initial random layout
  Sys::ProcGen::CellAutomataSystem cellauto_parser{ *m_window, *m_sprite_factory, *m_sound_bank, std::move( random_level ) };
  cellauto_parser.setRegistry( m_reg.get() );
  cellauto_parser.iterate( 5 );

  m_exit_sys->spawn_exit();

  // Reset the views early to prevent wild panning back to the start
  // position when the game starts rendering
  m_render_game_sys->init_views();
  m_sinkhole_sys->init_hazard_field();
  m_corruption_sys->init_hazard_field();
  m_wormhole_sys->spawn_wormhole( Sys::WormholeSystem::SpawnPhase::InitialSpawn );
}

// Teardown the engine
void Engine::exit_game()
{
  SPDLOG_INFO( "Exit game" );
  m_reg->clear();
}

void Engine::show_error_screen( const std::string &error_msg )
{
  std::ostringstream stack_trace;
  stack_trace << std::stacktrace::current();
  SPDLOG_CRITICAL( "Stack trace:\n{}", stack_trace.str() );

  sf::Font font;
  if ( !font.openFromFile( "res/fonts/tuffy.ttf" ) )
  {
    // Can't even show error screen, just exit
    return;
  }

  // Show only the error message on screen, not the stack trace
  std::string screen_message = "Fatal Error\n\n" + error_msg + "\n\nPress any key to exit" +
                               "\n\nFull stack trace saved to log.txt";

  sf::Text error_text( font, screen_message, 24 );
  error_text.setFillColor( sf::Color::White );
  error_text.setPosition( { 50, 50 } );

  m_window->clear( sf::Color::Black );
  m_window->draw( error_text );
  m_window->display();

  // Wait for key press
  while ( m_window->isOpen() )
  {
    while ( auto event = m_window->pollEvent() )
    {
      if ( event->is<sf::Event::Closed>() || event->is<sf::Event::KeyPressed>() )
      {
        m_window->close();
        return;
      }
    }
  }
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
  if ( not m_reg->view<Cmp::Persistent::GameState>()->empty() )
  {
    SPDLOG_WARN( "Game state entity already exists, skipping creation" );
    return;
  }
  SPDLOG_INFO( "Creating game state entity" );
  auto entity = m_reg->create();
  m_reg->emplace<Cmp::Persistent::GameState>( entity );
}

} // namespace ProceduralMaze