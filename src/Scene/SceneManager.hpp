#ifndef SCENE_SCENEMANAGER_HPP_
#define SCENE_SCENEMANAGER_HPP_

#include <Audio/SoundBank.hpp>
#include <Events/SceneManagerEvent.hpp>
#include <SFML/Graphics.hpp>

#include <Components/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <Scene/IScene.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/Systems.hpp>

#include <future>
#include <memory>
#include <vector>

namespace ProceduralMaze::Scene
{

class SceneManager
{
public:
  explicit SceneManager( sf::RenderWindow &w, Audio::SoundBank &sound_bank, Sys::SystemPtrs scene_di_sys_ptrs,
                         std::vector<Sys::BaseSystem *> reg_inject_system_ptrs, entt::dispatcher &nav_event_dispatcher )
      : m_window( w ),
        m_sound_bank( sound_bank ),
        m_reg_inject_system_ptrs( std::move( reg_inject_system_ptrs ) ),
        m_scene_di_sys_ptrs( scene_di_sys_ptrs ),
        m_nav_event_dispatcher( nav_event_dispatcher )
  {
    m_nav_event_dispatcher.sink<Events::SceneManagerEvent>().connect<&SceneManager::handle_events>( this );
  }

  void update( sf::Time dt );

  void push( std::unique_ptr<IScene> scene );
  void pop();

  void push_overlay( std::unique_ptr<IScene> scene );
  void pop_overlay();

  void replace( std::unique_ptr<IScene> scene );
  void replace_overlay( std::unique_ptr<IScene> scene );

  IScene *current();
  void gen_level();

  // void handle_request( SceneRequest req );

private:
  void inject_registry();

  template <typename Callable>
  void loading_screen( Callable &&callable, [[maybe_unused]] const sf::Texture &loading_texture )
  {
    Cmp::Font font( "res/fonts/tuffy.ttf" );
    sf::Text loading_text( font, "Loading", 48 );
    loading_text.setFillColor( sf::Color::White );
    loading_text.setPosition(
        { Sys::BaseSystem::kDisplaySize.x / 2.f - 50.f, Sys::BaseSystem::kDisplaySize.y / 2.f + 100.f } );

    sf::Clock clock;
    const float text_update_interval = 1.f; // 1 second between dot updates
    int dot_count = 0;

    // Launch the initialization task asynchronously
    auto future = std::async( std::launch::async, std::forward<Callable>( callable ) );

    // Run code while waiting for completion
    while ( future.wait_for( std::chrono::milliseconds( 16 ) ) != std::future_status::ready )
    {
      // Handle window events to prevent "not responding"
      while ( const std::optional event = m_window.pollEvent() )
      {
        if ( event->is<sf::Event::Closed>() )
        {
          m_window.close();
          return;
        }
      }

      // Update dots every second
      if ( clock.getElapsedTime().asSeconds() >= text_update_interval )
      {
        dot_count = ( dot_count + 1 ) % 4; // Cycle 0 -> 1 -> 2 -> 3 -> 0
        std::string dots( dot_count, '.' );
        loading_text.setString( "Loading" + dots );
        clock.restart();
      }

      m_window.clear( sf::Color::Black );
      m_window.draw( loading_text );
      m_window.display();
    }

    // Ensure any exceptions from the worker thread are propagated
    future.get();
  }

  void handle_events( const Events::SceneManagerEvent &event );

  void print_stack();

  //! @brief Non-owning reference to the OpenGL window
  sf::RenderWindow &m_window;
  Audio::SoundBank &m_sound_bank;

  std::vector<std::unique_ptr<IScene>> m_scenes;

  std::vector<Sys::BaseSystem *> m_reg_inject_system_ptrs;

  Sys::SystemPtrs m_scene_di_sys_ptrs;

  sf::Texture m_splash_texture{ "res/textures/splash.png" };

  entt::dispatcher &m_nav_event_dispatcher;
  std::recursive_mutex m_scene_mutex;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_SCENEMANAGER_HPP_