#include <CorruptionCell.hpp>
#include <Engine.hpp>

#include <MusicSystem.hpp>
#include <Persistent/MusicVolume.hpp>
#include <Persistent/ObstaclePushBack.hpp>
#include <SinkholeCell.hpp>

namespace ProceduralMaze {

Engine::Engine( ProceduralMaze::SharedEnttRegistry registry )
    : m_reg( std::move( registry ) ),
      m_sprite_factory( std::make_shared<Sprites::SpriteFactory>() ),
      m_player_sys( m_reg ),
      m_flood_sys( m_reg ),
      m_path_find_sys( m_reg ),
      m_npc_sys( m_reg ),
      m_collision_sys( m_reg ),
      m_render_game_sys( m_reg ),
      m_render_menu_sys( m_reg ),
      m_bomb_sys( m_reg ),
      m_sinkhole_sys( m_reg ),
      m_corruption_sys( m_reg ),
      m_title_music_sys( m_reg, "res/audio/title_music.mp3" ),
      m_underwater_sounds_sys( m_reg, "res/audio/underwater.wav" ),
      m_abovewater_sounds_sys( m_reg, "res/audio/footsteps.mp3" ),
      m_event_handler( m_reg )
{

  SPDLOG_INFO( "Engine Initiliasing... " );
  m_render_game_sys.window().setVerticalSyncEnabled( true );
  // m_render_game_sys.window().setFramerateLimit( 144 );

  // #ifdef _WIN32
  //   ::ShowWindow( m_render_game_sys.window().getNativeHandle(), SW_MAXIMIZE );
  // #endif

  // these need to be initialised by the time we get the
  // Cmp::Persistent::GameState::State::SETTINGS state
  m_event_handler.init_context();
  m_title_music_sys.init_context();
  m_bomb_sys.init_context();
  m_player_sys.init_context();
  m_flood_sys.init_context();
  m_collision_sys.init_context();
  m_npc_sys.init_context();
  m_path_find_sys.init_context();

  // setup ImGui here rather than RenderSystem classes to reduce white screen init time
  if ( not ImGui::SFML::Init( m_render_menu_sys.window() ) )
  {
    SPDLOG_CRITICAL( "ImGui-SFML initialization failed" );
    std::terminate();
  }

  // Set ImGui style
  ImGuiIO &io = ImGui::GetIO();
  io.FontGlobalScale = 1.5f;
  io.IniFilename = "res/imgui.ini";
  std::ignore = ImGui::SFML::UpdateFontTexture();

  SPDLOG_INFO( "Engine Initialisation Complete" );
}

bool Engine::run()
{
  sf::Clock deltaClock;

  /// MAIN LOOP BEGINS
  while ( m_render_game_sys.window().isOpen() )
  {
    sf::Time deltaTime = deltaClock.restart();
    m_title_music_sys.update_volume();

    auto &game_state = m_event_handler.get_game_state();

    switch ( game_state.current_state )
    {
      case Cmp::Persistent::GameState::State::MENU: {

        // process music playback
        // m_title_music_sys.update_music_playback( Sys::MusicSystem::Function::PLAY );

        m_render_menu_sys.render_title();
        m_event_handler.menu_state_handler( m_render_game_sys.window() );
        break;
      } // case MENU end

      case Cmp::Persistent::GameState::State::SETTINGS: {
        m_render_menu_sys.render_settings( deltaTime );
        m_event_handler.settings_state_handler( m_render_game_sys.window() );

        // make volume changes immediately audible

        break;
      } // case SETTINGS end

      case Cmp::Persistent::GameState::State::LOADING: {

        // wait for fade out to complete
        m_title_music_sys.start_music_fade_out();
        if ( m_title_music_sys.is_fading_out() )
        {
          m_title_music_sys.update_volume();
          break;
        }

        setup();
        game_state.current_state = Cmp::Persistent::GameState::State::PLAYING;
        SPDLOG_INFO( "Loading game...." );
        break;
      }

      case Cmp::Persistent::GameState::State::UNLOADING: {
        m_abovewater_sounds_sys.update_music_playback( Sys::MusicSystem::Function::STOP );
        m_underwater_sounds_sys.update_music_playback( Sys::MusicSystem::Function::STOP );
        teardown();
        game_state.current_state = Cmp::Persistent::GameState::State::MENU;
        SPDLOG_INFO( "Unloading game...." );
        break;
      }

      case Cmp::Persistent::GameState::State::PLAYING: {
        m_title_music_sys.update_music_playback( Sys::MusicSystem::Function::STOP );

        // check if player is underwater to start/stop underwater sounds
        auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Direction>();
        for ( auto [_, pc, dir_cmp] : player_view.each() )
        {
          if ( pc.underwater )
          {
            m_underwater_sounds_sys.update_music_playback( Sys::MusicSystem::Function::PLAY );
            m_abovewater_sounds_sys.update_music_playback( Sys::MusicSystem::Function::STOP );
          }
          else
          {
            m_underwater_sounds_sys.update_music_playback( Sys::MusicSystem::Function::STOP );
            // play footsteps only when player is moving
            if ( dir_cmp.x != 0.0f || dir_cmp.y != 0.0f )
            {
              m_abovewater_sounds_sys.update_music_playback( Sys::MusicSystem::Function::PLAY );
            }
            else { m_abovewater_sounds_sys.update_music_playback( Sys::MusicSystem::Function::STOP ); }
          }
        }

        m_event_handler.game_state_handler( m_render_game_sys.window() );

        m_player_sys.update( deltaTime );
        m_flood_sys.update();
        m_sinkhole_sys.update_hazard_field();
        m_corruption_sys.update_hazard_field();
        m_bomb_sys.update();

        m_collision_sys.check_npc_hazard_field_collision<Cmp::SinkholeCell>();
        m_collision_sys.check_npc_hazard_field_collision<Cmp::CorruptionCell>();
        m_collision_sys.check_end_zone_collision();
        m_collision_sys.check_loot_collision();
        m_collision_sys.check_bones_reanimation();

        m_collision_sys.update_obstacle_distances();

        auto player_entity = m_reg->view<Cmp::PlayableCharacter>().front();
        for ( auto [_ent, _sys] : m_reg->view<Cmp::System>().each() )
        {
          if ( _sys.collisions_enabled )
          {
            m_collision_sys.check_player_obstacle_collision();
            m_collision_sys.check_player_hazard_field_collision<Cmp::SinkholeCell>();
            m_collision_sys.check_player_hazard_field_collision<Cmp::CorruptionCell>();
            m_collision_sys.check_player_to_npc_collision();
          }
          if ( _sys.level_complete )
          {
            SPDLOG_INFO( "Level complete!" );
            game_state.current_state = Cmp::Persistent::GameState::State::GAMEOVER;
          }
        }

        m_path_find_sys.findPath( player_entity );
        m_npc_sys.lerp_movement( deltaTime );

        // did the player drown? Then end the game
        for ( auto [_, _pc] : m_reg->view<Cmp::PlayableCharacter>().each() )
        {
          if ( not _pc.alive ) { game_state.current_state = Cmp::Persistent::GameState::State::GAMEOVER; }
        }

        m_render_game_sys.render_game( deltaTime );
        break;
      } // case PLAYING end

      case Cmp::Persistent::GameState::State::PAUSED: {
        m_flood_sys.suspend();
        m_collision_sys.suspend();
        m_bomb_sys.suspend();

        while ( ( Cmp::Persistent::GameState::State::PAUSED == game_state.current_state ) and
                m_render_game_sys.window().isOpen() )
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

      case Cmp::Persistent::GameState::State::GAMEOVER: {
        m_abovewater_sounds_sys.update_music_playback( Sys::MusicSystem::Function::STOP );
        m_underwater_sounds_sys.update_music_playback( Sys::MusicSystem::Function::STOP );
        for ( auto [_, _pc] : m_reg->view<Cmp::PlayableCharacter>().each() )
        {
          if ( not _pc.alive ) { m_render_menu_sys.render_defeat_screen(); }
          else { m_render_menu_sys.render_victory_screen(); }
        }
        std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
        m_event_handler.game_over_state_handler( m_render_game_sys.window() );

        break;
      } // case GAME_OVER end

      case Cmp::Persistent::GameState::State::EXITING: {

        // wait for fade out to complete
        m_title_music_sys.start_music_fade_out();
        if ( m_title_music_sys.is_fading_out() )
        {
          m_title_music_sys.update_volume();
          break;
        }
        SPDLOG_INFO( "Terminating application...." );
        teardown();
        m_render_game_sys.window().close();
        std::terminate();
      }
    }

    // Update event dispatcher at end of frame
    Sys::BaseSystem::getEventDispatcher().update();

  } /// MAIN LOOP ENDS
  return false;
}

// Sets up ECS for the rest of the game
void Engine::setup()
{
  reginfo( "Pre-setup" );

  // Subscribe to NPC creation/death events
  std::ignore = Sys::BaseSystem::getEventDispatcher()
                    .sink<Events::NpcCreationEvent>()
                    .connect<&Sys::NpcSystem::on_npc_creation>( m_npc_sys );
  std::ignore = Sys::BaseSystem::getEventDispatcher()
                    .sink<Events::NpcDeathEvent>()
                    .connect<&Sys::NpcSystem::on_npc_death>( m_npc_sys );
  std::ignore = Sys::BaseSystem::getEventDispatcher()
                    .sink<Events::PlayerActionEvent>()
                    .connect<&Sys::BombSystem::on_player_action>( m_bomb_sys );

  // Cmp::Random::seed(123456789); // testing purposes

  m_reg->ctx().emplace<std::shared_ptr<Sprites::SpriteFactory>>( m_sprite_factory );

  m_render_game_sys.load_multisprites();
  add_system_entity();
  m_player_sys.add_player_entity();
  m_flood_sys.add_flood_water_entity();
  add_display_size( sf::Vector2u{ 1920, 1024 } );

  // create initial random game area with the required sprites
  std::unique_ptr<Sys::ProcGen::RandomLevelGenerator>
      random_level = std::make_unique<Sys::ProcGen::RandomLevelGenerator>( m_reg );

  // procedurally generate the game area from the initial random layout
  Sys::ProcGen::CellAutomataSystem cellauto_parser{ m_reg, std::move( random_level ) };
  cellauto_parser.iterate( 5 );

  // Reset the views early to prevent wild panning back to the start
  // position when the game starts rendering
  m_render_game_sys.init_views();
  m_sinkhole_sys.start_hazard_field();
  m_corruption_sys.start_hazard_field();

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