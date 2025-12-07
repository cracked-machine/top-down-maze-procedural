#ifndef SCENE_SCENEMANAGER_HPP_
#define SCENE_SCENEMANAGER_HPP_

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <Audio/SoundBank.hpp>
#include <Components/Font.hpp>
#include <Components/PlayerKeysCount.hpp>
#include <Components/PlayerRelicCount.hpp>
#include <SceneControl/ComponentTransfer.hpp>
#include <SceneControl/Events/SceneManagerEvent.hpp>
#include <SceneControl/IScene.hpp>
#include <SceneControl/SceneStack.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/SystemStore.hpp>

#include <future>
#include <memory>

namespace ProceduralMaze::Scene
{

class SceneManager
{
public:
  explicit SceneManager( sf::RenderWindow &w, Audio::SoundBank &sound_bank, Sys::SystemStore &system_store, entt::dispatcher &nav_event_dispatcher,
                         entt::dispatcher &scenemanager_event_dispatcher )
      : m_window( w ),
        m_sound_bank( sound_bank ),
        m_system_store( system_store ),
        m_nav_event_dispatcher( nav_event_dispatcher ),
        m_scenemanager_event_dispatcher( scenemanager_event_dispatcher )
  {
    m_scenemanager_event_dispatcher.sink<Events::SceneManagerEvent>().connect<&Scene::SceneManager::handle_events>( this );
  }

  // Update the current scene
  void update( sf::Time dt );

  // Render the current scene
  void push( std::unique_ptr<IScene> scene, ComponentTransfer::CopyRegistry retain_inventory = ComponentTransfer::CopyRegistry::SKIP );
  // Pop the current scene
  void pop( ComponentTransfer::CopyRegistry retain_inventory = ComponentTransfer::CopyRegistry::SKIP );

  // Push a new overlay scene - do not call on_exit() for the scene below this on the stack
  void push_overlay( std::unique_ptr<IScene> scene, ComponentTransfer::CopyRegistry retain_inventory = ComponentTransfer::CopyRegistry::SKIP );
  // Pop the current overlay scene - do not call on_enter() for the scene below this on the stack
  void pop_overlay( ComponentTransfer::CopyRegistry retain_inventory = ComponentTransfer::CopyRegistry::SKIP );

  // Replace the current scene with a new one
  void replace( std::unique_ptr<IScene> scene, ComponentTransfer::CopyRegistry retain_inventory = ComponentTransfer::CopyRegistry::SKIP );
  // Replace the current scene with a new one - do not call on_exit() for the replaced scene
  void replace_overlay( std::unique_ptr<IScene> scene, ComponentTransfer::CopyRegistry retain_inventory = ComponentTransfer::CopyRegistry::SKIP );

  // Get a pointer to the current active scene
  IScene *current();

  // Event handler for scene manager events
  void handle_events( const Events::SceneManagerEvent &event );

private:
  // Helper function to inject the current scene's registry into the system store
  void inject_current_scene_registry_into_systems();

  // Loading screen implementation
  template <typename Callable>
  void loading_screen( Callable &&callable, [[maybe_unused]] const sf::Texture &loading_texture )
  {
    Cmp::Font font( "res/fonts/tuffy.ttf" );
    sf::Text loading_text( font, "Loading", 48 );
    loading_text.setFillColor( sf::Color::White );
    loading_text.setPosition( { Sys::BaseSystem::kDisplaySize.x / 2.f - 50.f, Sys::BaseSystem::kDisplaySize.y / 2.f + 100.f } );

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

  //! @brief Non-owning reference to the OpenGL window
  sf::RenderWindow &m_window;

  //! @brief Non-owning reference to the sound bank
  Audio::SoundBank &m_sound_bank;

  //! @brief Non-owning reference to the system store
  Sys::SystemStore &m_system_store;

  // Scene stack managing active scenes
  SceneStack m_scene_stack;

  // Splash screen texture
  sf::Texture m_splash_texture{ "res/textures/splash.png" };

  //! @brief Non-owning reference to the navigation event dispatcher
  entt::dispatcher &m_nav_event_dispatcher;

  //! @brief Non-owning reference to the scene manager event dispatcher
  entt::dispatcher &m_scenemanager_event_dispatcher;

  //! @brief Used to transfer components from outgoing scene registry to an incoming scene registry
  ComponentTransfer m_cmp_transfer;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_SCENEMANAGER_HPP_