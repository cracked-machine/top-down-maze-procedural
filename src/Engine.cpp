#include <Components/CorruptionCell.hpp>
#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/Persistent/MusicVolume.hpp>
#include <Components/PlayerMortality.hpp>
#include <Components/SinkholeCell.hpp>
#include <Engine.hpp>
#include <Events/PauseClocksEvent.hpp>
#include <Events/ResumeClocksEvent.hpp>
#include <Scene/CryptScene.hpp>
#include <Scene/GraveyardScene.hpp>
#include <Scene/MainMenuScene.hpp>
#include <Scene/SceneManager.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/Threats/HazardFieldSystem.hpp>

#include <SFML/Window/WindowEnums.hpp>

#include <memory>
#include <stacktrace>

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

    m_scene_manager->push( std::make_unique<Scene::MainMenuScene>( *m_sound_bank, m_scene_di_sys_ptrs.persistent_sys,
                                                                   m_scene_di_sys_ptrs.render_menu_sys,
                                                                   m_scene_di_sys_ptrs.event_handler ) );
    sf::Clock globalFrameClock;

    /// MAIN LOOP BEGINS
    while ( m_window->isOpen() )
    {
      sf::Time globalDeltaTime = globalFrameClock.restart();
      m_scene_manager->update( globalDeltaTime );
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
  m_event_handler = std::make_unique<Sys::EventHandler>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_persistent_sys = std::make_unique<Sys::PersistentSystem>( *m_window, *m_sprite_factory, *m_sound_bank );

  m_sound_bank->init();

  m_render_game_sys = std::make_unique<Sys::RenderGameSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_player_sys = std::make_unique<Sys::PlayerSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_path_find_sys = std::make_unique<Sys::PathFindSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_npc_sys = std::make_unique<Sys::NpcSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_collision_sys = std::make_unique<Sys::CollisionSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_digging_sys = std::make_unique<Sys::DiggingSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_render_overlay_sys = std::make_unique<Sys::RenderOverlaySystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_render_player_sys = std::make_unique<Sys::RenderPlayerSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_bomb_sys = std::make_unique<Sys::BombSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_loot_sys = std::make_unique<Sys::LootSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_anim_sys = std::make_unique<Sys::AnimSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_sinkhole_sys = std::make_unique<Sys::SinkHoleHazardSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_corruption_sys = std::make_unique<Sys::CorruptionHazardSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_wormhole_sys = std::make_unique<Sys::WormholeSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_exit_sys = std::make_unique<Sys::ExitSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_footstep_sys = std::make_unique<Sys::FootstepSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_large_obstacle_sys = std::make_unique<Sys::LargeObstacleSystem>( *m_window, *m_sprite_factory, *m_sound_bank );
  m_random_level_sys = std::make_unique<Sys::ProcGen::RandomLevelGenerator>( *m_window, *m_sprite_factory,
                                                                             *m_sound_bank );
  m_cellauto_parser = std::make_unique<Sys::ProcGen::CellAutomataSystem>( *m_window, *m_sprite_factory, *m_sound_bank );

  // setup scene dependency injection pointers
  m_scene_di_sys_ptrs.render_menu_sys = m_render_menu_sys.get();
  m_scene_di_sys_ptrs.event_handler = m_event_handler.get();
  m_scene_di_sys_ptrs.render_game_sys = m_render_game_sys.get();
  m_scene_di_sys_ptrs.persistent_sys = m_persistent_sys.get();
  m_scene_di_sys_ptrs.player_sys = m_player_sys.get();
  m_scene_di_sys_ptrs.path_find_sys = m_path_find_sys.get();
  m_scene_di_sys_ptrs.npc_sys = m_npc_sys.get();
  m_scene_di_sys_ptrs.collision_sys = m_collision_sys.get();
  m_scene_di_sys_ptrs.digging_sys = m_digging_sys.get();
  m_scene_di_sys_ptrs.render_overlay_sys = m_render_overlay_sys.get();
  m_scene_di_sys_ptrs.render_player_sys = m_render_player_sys.get();
  m_scene_di_sys_ptrs.bomb_sys = m_bomb_sys.get();
  m_scene_di_sys_ptrs.anim_sys = m_anim_sys.get();
  m_scene_di_sys_ptrs.sinkhole_sys = m_sinkhole_sys.get();
  m_scene_di_sys_ptrs.corruption_sys = m_corruption_sys.get();
  m_scene_di_sys_ptrs.wormhole_sys = m_wormhole_sys.get();
  m_scene_di_sys_ptrs.exit_sys = m_exit_sys.get();
  m_scene_di_sys_ptrs.footstep_sys = m_footstep_sys.get();
  m_scene_di_sys_ptrs.large_obstacle_sys = m_large_obstacle_sys.get();
  m_scene_di_sys_ptrs.loot_sys = m_loot_sys.get();
  m_scene_di_sys_ptrs.random_level_sys = m_random_level_sys.get();
  m_scene_di_sys_ptrs.cellauto_parser = m_cellauto_parser.get();

  // setup registry injection pointers
  m_reg_inject_system_ptrs.push_back( m_render_menu_sys.get() );
  m_reg_inject_system_ptrs.push_back( m_event_handler.get() );
  m_reg_inject_system_ptrs.push_back( m_render_game_sys.get() );
  m_reg_inject_system_ptrs.push_back( m_persistent_sys.get() );
  m_reg_inject_system_ptrs.push_back( m_player_sys.get() );
  m_reg_inject_system_ptrs.push_back( m_path_find_sys.get() );
  m_reg_inject_system_ptrs.push_back( m_npc_sys.get() );
  m_reg_inject_system_ptrs.push_back( m_collision_sys.get() );
  m_reg_inject_system_ptrs.push_back( m_digging_sys.get() );
  m_reg_inject_system_ptrs.push_back( m_render_overlay_sys.get() );
  m_reg_inject_system_ptrs.push_back( m_render_player_sys.get() );
  m_reg_inject_system_ptrs.push_back( m_bomb_sys.get() );
  m_reg_inject_system_ptrs.push_back( m_anim_sys.get() );
  m_reg_inject_system_ptrs.push_back( m_sinkhole_sys.get() );
  m_reg_inject_system_ptrs.push_back( m_corruption_sys.get() );
  m_reg_inject_system_ptrs.push_back( m_wormhole_sys.get() );
  m_reg_inject_system_ptrs.push_back( m_exit_sys.get() );
  m_reg_inject_system_ptrs.push_back( m_footstep_sys.get() );
  m_reg_inject_system_ptrs.push_back( m_large_obstacle_sys.get() );
  m_reg_inject_system_ptrs.push_back( m_loot_sys.get() );
  m_reg_inject_system_ptrs.push_back( m_random_level_sys.get() );
  m_reg_inject_system_ptrs.push_back( m_cellauto_parser.get() );

  // create the scene manager now that all systems are initialized
  m_scene_manager = std::make_unique<Scene::SceneManager>( *m_window, *m_sound_bank, m_scene_di_sys_ptrs,
                                                           m_reg_inject_system_ptrs );

  SPDLOG_INFO( "Lazy initialization of systems complete" );
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

} // namespace ProceduralMaze