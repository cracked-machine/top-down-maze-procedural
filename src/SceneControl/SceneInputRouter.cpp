#include <Components/Exit.hpp>
#include <Components/Persistent/ExitKeyRequirement.hpp>
#include <Components/Persistent/MaxNumAltars.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerCandlesCount.hpp>
#include <Components/PlayerHealth.hpp>
#include <Components/PlayerKeysCount.hpp>
#include <Components/PlayerMortality.hpp>
#include <Components/PlayerRelicCount.hpp>
#include <Events/SaveSettingsEvent.hpp>
#include <Events/UnlockDoorEvent.hpp>
#include <SceneControl/Events/ProcessGameoverSceneInputEvent.hpp>
#include <SceneControl/Events/ProcessGraveyardSceneInputEvent.hpp>
#include <SceneControl/Events/ProcessLevelCompleteSceneInputEvent.hpp>
#include <SceneControl/Events/ProcessPausedMenuSceneInputEvent.hpp>
#include <SceneControl/Events/ProcessSettingsMenuSceneInputEvent.hpp>
#include <SceneControl/Events/ProcessTitleSceneInputEvent.hpp>
#include <SceneControl/Events/SceneManagerEvent.hpp>
#include <SceneControl/SceneInputRouter.hpp>

namespace ProceduralMaze::Sys
{

SceneInputRouter::SceneInputRouter( entt::registry &reg, sf::RenderWindow &m_window, Sprites::SpriteFactory &sprite_factory,
                                    Audio::SoundBank &sound_bank, entt::dispatcher &nav_event_dispatcher,
                                    entt::dispatcher &scenemanager_event_dispatcher )
    : Sys::BaseSystem( reg, m_window, sprite_factory, sound_bank ),
      m_nav_event_dispatcher( nav_event_dispatcher ),
      m_scenemanager_event_dispatcher( scenemanager_event_dispatcher )
{
  // clang-format off
  m_nav_event_dispatcher.sink<Events::ProcessTitleSceneInputEvent>().connect<&SceneInputRouter::title_scene_input_handler>( this );
  m_nav_event_dispatcher.sink<Events::ProcessSettingsMenuSceneInputEvent>().connect<&SceneInputRouter::settings_scene_state_handler>( this );
  m_nav_event_dispatcher.sink<Events::ProcessGraveyardSceneInputEvent>().connect<&SceneInputRouter::graveyard_scene_state_handler>(this );
  m_nav_event_dispatcher.sink<Events::ProcessPausedMenuSceneInputEvent>().connect<&SceneInputRouter::paused_scene_state_handler>(this );
  m_nav_event_dispatcher.sink<Events::ProcessGameoverSceneInputEvent>().connect<&SceneInputRouter::game_over_scene_state_handler>(this );
  m_nav_event_dispatcher.sink<Events::ProcessLevelCompleteSceneInputEvent>().connect<&SceneInputRouter::level_complete_scene_state_handler>( this );
  // clang-format on
}

void SceneInputRouter::title_scene_input_handler()
{
  using namespace sf::Keyboard;
  while ( const std::optional event = m_window.pollEvent() )
  {
    ImGui::SFML::ProcessEvent( m_window, *event );
    if ( event->is<sf::Event::Closed>() ) { m_window.close(); }
    else if ( const auto *resized = event->getIf<sf::Event::Resized>() )
    {
      sf::FloatRect visibleArea( { 0.f, 0.f }, sf::Vector2f( resized->size ) );
      m_window.setView( sf::View( visibleArea ) );
    }
    else if ( const auto *keyPressed = event->getIf<sf::Event::KeyPressed>() )
    {
      if ( keyPressed->scancode == sf::Keyboard::Scancode::Enter )
      {
        m_scenemanager_event_dispatcher.enqueue( Events::SceneManagerEvent( Events::SceneManagerEvent::Type::START_GAME ) );
      }
      else if ( keyPressed->scancode == sf::Keyboard::Scancode::Q )
      {
        m_scenemanager_event_dispatcher.enqueue( Events::SceneManagerEvent( Events::SceneManagerEvent::Type::EXIT_GAME ) );
      }
      else if ( keyPressed->scancode == sf::Keyboard::Scancode::S )
      {
        m_scenemanager_event_dispatcher.enqueue( Events::SceneManagerEvent( Events::SceneManagerEvent::Type::SETTINGS_MENU ) );
      }
    }
  }
}

void SceneInputRouter::settings_scene_state_handler()
{
  using namespace sf::Keyboard;
  while ( const std::optional event = m_window.pollEvent() )
  {
    ImGui::SFML::ProcessEvent( m_window, *event );
    if ( event->is<sf::Event::Closed>() ) { m_window.close(); }
    else if ( const auto *resized = event->getIf<sf::Event::Resized>() )
    {
      sf::FloatRect visibleArea( { 0.f, 0.f }, sf::Vector2f( resized->size ) );
      m_window.setView( sf::View( visibleArea ) );
    }
    else if ( const auto *keyPressed = event->getIf<sf::Event::KeyPressed>() )
    {
      if ( keyPressed->scancode == sf::Keyboard::Scancode::Escape )
      {
        m_scenemanager_event_dispatcher.enqueue( Events::SceneManagerEvent( Events::SceneManagerEvent::Type::EXIT_SETTINGS_MENU ) );
      }
    }
  }
}

void SceneInputRouter::graveyard_scene_state_handler()
{

  using namespace sf::Keyboard;
  while ( const std::optional event = m_window.pollEvent() )
  {
    ImGui::SFML::ProcessEvent( m_window, *event );
    if ( event->is<sf::Event::Closed>() ) { m_window.close(); }
    else if ( const auto *resized = event->getIf<sf::Event::Resized>() )
    {
      sf::FloatRect visibleArea( { 0.f, 0.f }, sf::Vector2f( resized->size ) );
      m_window.setView( sf::View( visibleArea ) );
    }
    else if ( const auto *keyReleased = event->getIf<sf::Event::KeyReleased>() )
    {
      if ( keyReleased->scancode == sf::Keyboard::Scancode::F1 )
      {
        for ( auto [_entt, _sys] : getReg().view<Cmp::System>().each() )
        {
          _sys.collisions_enabled = not _sys.collisions_enabled;
          SPDLOG_INFO( "Collisions are now {}", _sys.collisions_enabled ? "ENABLED" : "DISABLED" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F2 )
      {
        for ( auto [_entt, _sys] : getReg().view<Cmp::System>().each() )
        {
          _sys.pause_flood = not _sys.pause_flood;
          SPDLOG_INFO( "Pause flood is now {}", _sys.pause_flood ? "ENABLED" : "DISABLED" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F3 )
      {
        for ( auto [_entt, _sys] : getReg().view<Cmp::System>().each() )
        {
          _sys.show_path_distances = not _sys.show_path_distances;
          SPDLOG_INFO( "Show player distances is now {}", _sys.show_path_distances ? "ENABLED" : "DISABLED" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F4 )
      {
        for ( auto [_entt, _sys] : getReg().view<Cmp::System>().each() )
        {
          _sys.show_armed_obstacles = not _sys.show_armed_obstacles;
          SPDLOG_INFO( "Show armed obstacles is now {}", _sys.show_armed_obstacles ? "ENABLED" : "DISABLED" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F5 )
      {
        for ( auto [_entt, _sys] : getReg().view<Cmp::System>().each() )
        {
          _sys.show_debug_stats = not _sys.show_debug_stats;
          SPDLOG_INFO( "Show debug stats is now {}", _sys.show_debug_stats ? "ENABLED" : "DISABLED" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F10 )
      {
        // UnlockDoorEvent will check player key count so give player 3 keys first
        auto player_key_view = getReg().view<Cmp::PlayerKeysCount>();
        for ( auto [pkey_count_entity, pkey_count_cmp] : player_key_view.each() )
        {
          pkey_count_cmp.increment_count( get_persistent_component<Cmp::Persistent::ExitKeyRequirement>().get_value() );
        }
        auto exit_cmp = getReg().view<Cmp::Exit>();
        for ( auto [exit_entt, exit_cmp] : exit_cmp.each() )
        {
          // exit already unlocked
          if ( exit_cmp.m_locked == false ) continue;
        }
        get_systems_event_queue().trigger( Events::UnlockDoorEvent() );
        SPDLOG_INFO( "Player cheated and unlocked exit" );
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F11 )
      {
        // dont set PlayerMortality::State directly, instead update health/death_progress and let the PlayerSystem logic
        // handle it
        for ( auto [entity, pc_mort_cmp, pc_health_cmp] : getReg().view<Cmp::PlayerMortality, Cmp::PlayerHealth>().each() )
        {
          pc_health_cmp.health = 0;
          pc_mort_cmp.death_progress = 1.0f;
          SPDLOG_INFO( "Player committed suicide" );
        }
      }

      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F12 )
      {
        for ( auto [_ent, _sys] : getReg().view<Cmp::System>().each() )
        {
          _sys.level_complete = true;
          SPDLOG_INFO( "Level complete (player cheated)" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::M )
      {
        for ( auto [_ent, _sys] : getReg().view<Cmp::System>().each() )
        {
          _sys.minimap_enabled = not _sys.minimap_enabled;
          SPDLOG_INFO( "Minimap enabled (player cheated)" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad1 )
      {
        for ( auto [pc_entity, pc_candle_count_cmp] : getReg().view<Cmp::PlayerCandlesCount>().each() )
        {
          pc_candle_count_cmp.increment_count( 1 );
          SPDLOG_INFO( "Player gained a candle (player cheated)" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad2 )
      {
        for ( auto [pc_entity, pc_key_count_cmp] : getReg().view<Cmp::PlayerKeysCount>().each() )
        {
          pc_key_count_cmp.increment_count( 1 );
          SPDLOG_INFO( "Player gained a key (player cheated)" );
          get_systems_event_queue().trigger( Events::UnlockDoorEvent() );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad3 )
      {
        for ( auto [pc_entity, pc_cmp, pc_health_cmp] : getReg().view<Cmp::PlayableCharacter, Cmp::PlayerHealth>().each() )
        {
          pc_health_cmp.health = std::clamp( 10, pc_health_cmp.health + 10, 100 );
          SPDLOG_INFO( "Player gained health (player cheated)" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad4 )
      {
        for ( auto [pc_entity, pc_cmp, pc_relic_count_cmp] : getReg().view<Cmp::PlayableCharacter, Cmp::PlayerRelicCount>().each() )
        {
          pc_relic_count_cmp.increment_count( 1 );
          SPDLOG_INFO( "Player gained a relic (player cheated)" );
        }
      }

      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Escape )
      {
        m_scenemanager_event_dispatcher.enqueue( Events::SceneManagerEvent( Events::SceneManagerEvent::Type::QUIT_GAME ) );
      }
    }
    else if ( const auto *keyPressed = event->getIf<sf::Event::KeyPressed>() )
    {
      if ( keyPressed->scancode == sf::Keyboard::Scancode::P )
      {
        m_scenemanager_event_dispatcher.enqueue( Events::SceneManagerEvent( Events::SceneManagerEvent::Type::PAUSE_GAME ) );
      }
    }
  }

  // allow multiple changes to the direction vector, otherwise we get a delayed slurred movement
  auto player_direction_view = getReg().view<Cmp::PlayableCharacter, Cmp::Direction>();
  for ( auto [entity, player, direction] : player_direction_view.each() )
  {
    direction.x = 0;
    direction.y = 0;
    if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Key::W ) ) { direction.y = -1; } // move player up
    if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Key::A ) ) { direction.x = -1; } // move player left
    if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Key::D ) ) { direction.x = 1; }  // move player right
    if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Key::S ) ) { direction.y = 1; }  // move player down
  }

  if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Key::Space ) )
  {
    get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::DROP_BOMB ) );
  }
  if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Key::E ) )
  {
    get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::ACTIVATE ) );
  }
  if ( sf::Mouse::isButtonPressed( sf::Mouse::Button::Left ) )
  {
    get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::DIG ) );
  }
}

void SceneInputRouter::paused_scene_state_handler()
{

  using namespace sf::Keyboard;
  while ( const std::optional event = m_window.pollEvent() )
  {
    ImGui::SFML::ProcessEvent( m_window, *event );
    if ( event->is<sf::Event::Closed>() ) { m_window.close(); }
    else if ( const auto *resized = event->getIf<sf::Event::Resized>() )
    {
      sf::FloatRect visibleArea( { 0.f, 0.f }, sf::Vector2f( resized->size ) );
      m_window.setView( sf::View( visibleArea ) );
    }
    else if ( const auto *keyPressed = event->getIf<sf::Event::KeyPressed>() )
    {
      if ( keyPressed->scancode == sf::Keyboard::Scancode::P )
      {
        m_scenemanager_event_dispatcher.enqueue( Events::SceneManagerEvent( Events::SceneManagerEvent::Type::RESUME_GAME ) );
      }
    }
  }
}

void SceneInputRouter::game_over_scene_state_handler()
{
  using namespace sf::Keyboard;
  while ( const std::optional event = m_window.pollEvent() )
  {
    ImGui::SFML::ProcessEvent( m_window, *event );
    if ( event->is<sf::Event::Closed>() ) { m_window.close(); }
    else if ( const auto *resized = event->getIf<sf::Event::Resized>() )
    {
      sf::FloatRect visibleArea( { 0.f, 0.f }, sf::Vector2f( resized->size ) );
      m_window.setView( sf::View( visibleArea ) );
    }
    else if ( const auto *keyPressed = event->getIf<sf::Event::KeyPressed>() )
    {
      if ( keyPressed->scancode == sf::Keyboard::Scancode::R )
      {
        m_scenemanager_event_dispatcher.enqueue( Events::SceneManagerEvent( Events::SceneManagerEvent::Type::RETURN_TO_TITLE ) );
      }
    }
  }
}

void SceneInputRouter::level_complete_scene_state_handler()
{
  using namespace sf::Keyboard;
  while ( const std::optional event = m_window.pollEvent() )
  {
    ImGui::SFML::ProcessEvent( m_window, *event );
    if ( event->is<sf::Event::Closed>() ) { m_window.close(); }
    else if ( const auto *resized = event->getIf<sf::Event::Resized>() )
    {
      sf::FloatRect visibleArea( { 0.f, 0.f }, sf::Vector2f( resized->size ) );
      m_window.setView( sf::View( visibleArea ) );
    }
    else if ( const auto *keyPressed = event->getIf<sf::Event::KeyPressed>() )
    {
      if ( keyPressed->scancode == sf::Keyboard::Scancode::R )
      {
        m_scenemanager_event_dispatcher.enqueue( Events::SceneManagerEvent( Events::SceneManagerEvent::Type::RETURN_TO_TITLE ) );
      }
    }
  }
}

} // namespace ProceduralMaze::Sys