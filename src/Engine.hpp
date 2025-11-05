#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__

#include <Audio/SoundBank.hpp>
#include <Systems/LootSystem.hpp>
#include <entt/entity/registry.hpp>

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <spdlog/spdlog.h>

#include <Components/Direction.hpp>
#include <Components/DisplaySize.hpp>
#include <Components/Font.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/FuseDelay.hpp>
#include <Components/Persistent/GameState.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Position.hpp>
#include <Components/Random.hpp>
#include <Components/System.hpp>
#include <Components/WaterLevel.hpp>
#include <EventHandler.hpp>
#include <Sprites/TileMap.hpp>
#include <Systems/AnimSystem.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/CollisionSystem.hpp>
#include <Systems/DiggingSystem.hpp>
#include <Systems/ExitSystem.hpp>
#include <Systems/LargeObstacleSystem.hpp>
#include <Systems/PathFindSystem.hpp>
#include <Systems/PersistentSystem.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/ProcGen/CellAutomataSystem.hpp>
#include <Systems/ProcGen/RandomLevelGenerator.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/Render/RenderMenuSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Systems/Threats/BombSystem.hpp>
#include <Systems/Threats/FloodSystem.hpp>
#include <Systems/Threats/HazardFieldSystem.hpp>
#include <Systems/Threats/NpcSystem.hpp>
#include <Systems/Threats/WormholeSystem.hpp>

#include <future>
#include <memory>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace ProceduralMaze {

// Singleton-like Engine class:
// - Manages game state
// - Initialise/Teardown ECS registry
// - Owns system objects and SpriteFactory
// - Runs the main game loop
// - Non-copyable/movable
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
  // setup the new game environment/state
  void enter_game();
  // teardown the game environment/state
  void exit_game();
  // show error screen with message
  void show_error_screen( const std::string &error_msg );
  // print registry info for debugging
  void reginfo( std::string msg = "" );
  // add display size component to the registry
  void add_display_size( const sf::Vector2u &size );
  // add system component to the registry
  void add_system_entity();
  // add game state component to the registry
  void add_game_state_entity();

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
      while ( const std::optional event = m_window->pollEvent() )
      {
        if ( event->is<sf::Event::Closed>() )
        {
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

  // Create the ECS registry
  ProceduralMaze::SharedEnttRegistry m_reg{ std::make_shared<entt::basic_registry<entt::entity>>() };

  // Create the opengl window
  std::unique_ptr<sf::RenderWindow> m_window = std::make_unique<sf::RenderWindow>(
      sf::VideoMode( Sys::BaseSystem::kDisplaySize ), "ProceduralMaze", sf::State::Fullscreen );

  // create MultiSprite resources
  std::unique_ptr<Sprites::SpriteFactory> m_sprite_factory;

  std::unique_ptr<Audio::SoundBank> m_sound_bank = std::make_unique<Audio::SoundBank>();

  //  ECS Systems
  std::unique_ptr<Sys::RenderMenuSystem> m_render_menu_sys;
  std::unique_ptr<Sys::EventHandler> m_event_handler;
  std::unique_ptr<Sys::RenderGameSystem> m_render_game_sys;
  std::unique_ptr<Sys::PersistentSystem> m_persistent_sys;
  std::unique_ptr<Sys::PlayerSystem> m_player_sys;
  std::unique_ptr<Sys::FloodSystem> m_flood_sys;
  std::unique_ptr<Sys::PathFindSystem> m_path_find_sys;
  std::unique_ptr<Sys::NpcSystem> m_npc_sys;
  std::unique_ptr<Sys::CollisionSystem> m_collision_sys;
  std::unique_ptr<Sys::DiggingSystem> m_digging_sys;
  std::unique_ptr<Sys::RenderOverlaySystem> m_render_overlay_sys;
  std::unique_ptr<Sys::BombSystem> m_bomb_sys;
  std::unique_ptr<Sys::AnimSystem> m_anim_sys;
  std::unique_ptr<Sys::SinkHoleHazardSystem> m_sinkhole_sys;
  std::unique_ptr<Sys::CorruptionHazardSystem> m_corruption_sys;
  std::unique_ptr<Sys::WormholeSystem> m_wormhole_sys;
  std::unique_ptr<Sys::ExitSystem> m_exit_sys;
  std::unique_ptr<Sys::FootstepSystem> m_footstep_sys;
  std::unique_ptr<Sys::LargeObstacleSystem> m_large_obstacle_sys;
  std::unique_ptr<Sys::LootSystem> m_loot_sys;

  // restrict the path tracking data update to every 0.1 seconds (optimization)
  const sf::Time m_obstacle_distance_update_interval{ sf::milliseconds( 100 ) };
  // path tracking timer (optimization)
  sf::Clock m_obstacle_distance_timer;

  // flag to indicate when systems have finished initializing; used by loading_screen()
  bool m_systems_initialized = false;
};

} // namespace ProceduralMaze

#endif // __ENGINE_HPP__