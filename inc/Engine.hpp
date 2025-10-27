#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__

#include <spdlog/spdlog.h>

#include <AnimSystem.hpp>
#include <BaseSystem.hpp>
#include <BombSystem.hpp>
#include <Components/DisplaySize.hpp>
#include <Components/Font.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Position.hpp>
#include <Components/Random.hpp>
#include <Components/System.hpp>
#include <DiggingSystem.hpp>
#include <Direction.hpp>
#include <EventHandler.hpp>
#include <ExitSystem.hpp>
#include <FloodSystem.hpp>
#include <HazardFieldSystem.hpp>
#include <MusicSystem.hpp>
#include <NpcSystem.hpp>
#include <PathFindSystem.hpp>
#include <Persistent/FuseDelay.hpp>
#include <Persistent/GameState.hpp>
#include <PersistentSystem.hpp>
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
#include <Sprites/TileMap.hpp>
#include <Systems/CollisionSystem.hpp>
#include <Systems/ProcGen/CellAutomataSystem.hpp>
#include <Systems/RenderSystem.hpp>
#include <WaterLevel.hpp>
#include <WormholeSystem.hpp>
#include <entt/entity/registry.hpp>
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
  explicit Engine( ProceduralMaze::SharedEnttRegistry registry );

  Engine( const Engine & ) = delete;
  Engine &operator=( const Engine & ) = delete;
  Engine( Engine && ) = delete;
  Engine &operator=( Engine && ) = delete;

  bool run();

private:
  // ECS registry
  ProceduralMaze::SharedEnttRegistry m_reg;

  Sprites::TitleScreenShader m_title_screen_shader{ "res/shaders/TitleScreen.frag", Sys::BaseSystem::kDisplaySize };

  std::unique_ptr<sf::RenderWindow> m_window = std::make_unique<sf::RenderWindow>(
      sf::VideoMode( Sys::BaseSystem::kDisplaySize ), "ProceduralMaze", sf::State::Fullscreen );

  // init this first since it registers core settings
  Sys::PersistentSystem m_persistent_sys;

  // creates and manages MultiSprite resources
  std::shared_ptr<Sprites::SpriteFactory> m_sprite_factory = std::make_shared<Sprites::SpriteFactory>();

  //  ECS Systems

  Sys::PlayerSystem m_player_sys;
  Sys::FloodSystem m_flood_sys;
  Sys::PathFindSystem m_path_find_sys;
  Sys::NpcSystem m_npc_sys;
  Sys::CollisionSystem m_collision_sys;
  Sys::DiggingSystem m_digging_sys;
  Sys::RenderGameSystem m_render_game_sys;
  Sys::RenderOverlaySystem m_render_overlay_sys;
  Sys::RenderMenuSystem m_render_menu_sys;
  Sys::BombSystem m_bomb_sys;
  Sys::AnimSystem m_anim_sys;
  Sys::SinkHoleHazardSystem m_sinkhole_sys;
  Sys::CorruptionHazardSystem m_corruption_sys;
  Sys::WormholeSystem m_wormhole_sys;
  Sys::ExitSystem m_exit_sys;
  Sys::FootstepSystem m_footstep_sys;
  Sys::MusicSystem m_title_music_sys;
  Sys::MusicSystem m_underwater_sounds_sys;
  Sys::MusicSystem m_abovewater_sounds_sys;

  // SFML keyboard/mouse event handler
  ProceduralMaze::EventHandler m_event_handler;

  // sets up ECS just enough to let the statemachine work
  void bootstrap();

  // Sets up ECS for the rest of the game
  void setup();

  // Teardown the engine
  void teardown();

  void reginfo( std::string msg = "" );
  void add_display_size( const sf::Vector2u &size );
  void add_system_entity();
  void add_game_state_entity();

  // restrict the path tracking data update to every 0.1 seconds (optimization)
  const sf::Time m_obstacle_distance_update_interval{ sf::milliseconds( 100 ) };
  sf::Clock m_obstacle_distance_timer;
};

} // namespace ProceduralMaze

#endif // __ENGINE_HPP__