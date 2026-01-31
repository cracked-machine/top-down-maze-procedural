#include <Components/Hazard/CorruptionCell.hpp>
#include <Components/Hazard/SinkholeCell.hpp>
#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/Persistent/MusicVolume.hpp>
#include <Components/Player/PlayerMortality.hpp>
#include <Engine.hpp>
#include <Events/PauseClocksEvent.hpp>
#include <Events/ResumeClocksEvent.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <SceneControl/SceneManager.hpp>
#include <SceneControl/Scenes/CryptScene.hpp>
#include <SceneControl/Scenes/GraveyardScene.hpp>
#include <SceneControl/Scenes/TitleScene.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/SystemStore.hpp>
#include <Systems/Threats/HazardFieldSystem.hpp>
#include <Systems/Threats/HazardFieldSystemImpl.hpp>
#include <imgui-SFML.h>

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
    ImGui::SFML::Shutdown();
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

    m_scene_manager->push( std::make_unique<Scene::TitleScene>( *m_sound_bank, *m_system_store, m_nav_event_dispatcher ) );
    sf::Clock globalFrameClock;

    /// MAIN LOOP BEGINS
    while ( m_window->isOpen() )
    {
      sf::Time globalDeltaTime = globalFrameClock.restart();
      m_scene_manager->update( globalDeltaTime );
      Sys::BaseSystem::get_systems_event_queue().update();

      // catch the resize events
      while ( const std::optional event = m_window->pollEvent() )
      {
        if ( const auto *resized = event->getIf<sf::Event::Resized>() )
        {
          // update the view to the new size of the window
          sf::FloatRect visibleArea( { 0.f, 0.f }, sf::Vector2f( resized->size ) );
          m_window->setView( sf::View( visibleArea ) );
        }
      }
    } /// MAIN LOOP ENDS
  } catch ( const std::exception &e )
  {
    SPDLOG_CRITICAL( "Unhandled exception in Engine::run(): {}", e.what() );
    show_error_screen( e.what() );
    return false;
  } catch ( ... )
  {
    SPDLOG_CRITICAL( "Unhandled unknown exception in Engine::run()" );
    show_error_screen( "An unknown error has occurred. Please check log." );
    return false;
  }
  return true; // exit game
}

void Engine::init_systems()
{
  m_sprite_factory = std::make_unique<Sprites::SpriteFactory>();
  m_sprite_factory->init();
  m_sound_bank->init();
  m_system_store = std::make_unique<Sys::SystemStore>( *m_window, *m_sprite_factory, *m_sound_bank, m_nav_event_dispatcher,
                                                       m_scenemanager_event_queue );
  m_scene_manager = std::make_unique<Scene::SceneManager>( *m_window, *m_sound_bank, *m_system_store, m_nav_event_dispatcher,
                                                           m_scenemanager_event_queue, *m_sprite_factory );

  SPDLOG_DEBUG( "Lazy initialization of systems complete" );
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
  std::string screen_message = "Fatal Error\n\n" + error_msg + "\n\nPress any key to exit" + "\n\nFull stack trace saved to log.txt";

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
        ImGui::SFML::Shutdown();
        m_window->close();
        return;
      }
    }
  }
}

} // namespace ProceduralMaze