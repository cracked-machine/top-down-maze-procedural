#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__

#include "imgui-SFML.h"
#include <SFML/Audio/Sound.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <entt/entity/fwd.hpp>

#include <Audio/SoundBank.hpp>
#include <Components/Font.hpp>
#include <SceneControl/SceneManager.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/SystemStore.hpp>

#include <future>
#include <memory>
#include <string>

namespace ProceduralMaze
{

class Engine
{
public:
  explicit Engine();

  Engine( const Engine & ) = delete;
  Engine &operator=( const Engine & ) = delete;
  Engine( Engine && ) = delete;
  Engine &operator=( Engine && ) = delete;

  // main loop
  bool run();

private:
  // initialize all ECS systems
  void init_systems();

  // show error screen with message
  void show_error_screen( const std::string &error_msg );

  // display a loading screen while executing the provided callable
  template <typename Callable>
  void loading_screen( Callable &&callable, [[maybe_unused]] const sf::Texture &loading_texture )
  {
    Cmp::Font font( "res/fonts/tuffy.ttf" );
    sf::Text loading_text( font, "Loading", 48 );
    loading_text.setFillColor( sf::Color::White );
    // use fallback resolution for loading screen position since we dont have registry access at this point
    loading_text.setPosition( { Constants::kFallbackDisplaySize.x / 2.f - 50.f, Constants::kFallbackDisplaySize.y / 2.f + 100.f } );

    sf::Clock clock;
    const float text_update_interval = 1.f; // 1 second between dot updates
    int dot_count = 0;

    // Launch the initialization task asynchronously
    auto future = std::async( std::launch::async, std::forward<Callable>( callable ) );

    // Run code while waiting for completion
    while ( future.wait_for( std::chrono::milliseconds( 16 ) ) != std::future_status::ready )
    {
      // Handle window events to prevent "not responding"
      while ( const std::optional event = m_window->pollEvent() )
      {
        if ( event->is<sf::Event::Closed>() )
        {
          ImGui::SFML::Shutdown();
          m_window->close();
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

      m_window->clear( sf::Color::Black );
      m_window->draw( loading_text );
      m_window->display();
    }

    // Ensure any exceptions from the worker thread are propagated
    future.get();
  }

  // load the splash texture first
  sf::Texture m_splash_texture{ "res/textures/splash.png" };

  // Create the opengl window
  // use fallback resolution for loading screen position since we dont have registry access at this point
  std::unique_ptr<sf::RenderWindow> m_window = std::make_unique<sf::RenderWindow>( sf::VideoMode( Constants::kFallbackDisplaySize ), "ProceduralMaze",
                                                                                   sf::State::Fullscreen );

  // create MultiSprite resources
  std::unique_ptr<Sprites::SpriteFactory> m_sprite_factory;
  std::unique_ptr<Audio::SoundBank> m_sound_bank = std::make_unique<Audio::SoundBank>();

  std::unique_ptr<Sys::SystemStore> m_system_store;
  std::unique_ptr<Scene::SceneManager> m_scene_manager;

  entt::dispatcher m_nav_event_dispatcher;
  entt::dispatcher m_scenemanager_event_queue;
};

} // namespace ProceduralMaze

#endif // __ENGINE_HPP__