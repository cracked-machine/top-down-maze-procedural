#include <Components/Direction.hpp>
#include <Components/Exit.hpp>
#include <Components/Persistent/ExitKeyRequirement.hpp>
#include <Components/Persistent/GameState.hpp>
#include <Components/Persistent/MaxNumAltars.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerCadaverCount.hpp>
#include <Components/PlayerHealth.hpp>
#include <Components/PlayerKeysCount.hpp>
#include <Components/PlayerMortality.hpp>
#include <Components/Position.hpp>
#include <Components/System.hpp>
#include <Events/CryptRoomEvent.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Events/PlayerMortalityEvent.hpp>
#include <Events/SaveSettingsEvent.hpp>
#include <Events/UnlockDoorEvent.hpp>
#include <SceneControl/Events/ProcessCryptSceneInputEvent.hpp>
#include <SceneControl/Events/ProcessGameoverSceneInputEvent.hpp>
#include <SceneControl/Events/ProcessGraveyardSceneInputEvent.hpp>
#include <SceneControl/Events/ProcessLevelCompleteSceneInputEvent.hpp>
#include <SceneControl/Events/ProcessPausedMenuSceneInputEvent.hpp>
#include <SceneControl/Events/ProcessSettingsMenuSceneInputEvent.hpp>
#include <SceneControl/Events/ProcessTitleSceneInputEvent.hpp>
#include <SceneControl/Events/SceneManagerEvent.hpp>
#include <SceneControl/SceneInputRouter.hpp>
#include <Systems/PersistSystem.hpp>
#include <Utils/Utils.hpp>

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
  m_nav_event_dispatcher.sink<Events::ProcessCryptSceneInputEvent>().connect<&SceneInputRouter::crypt_scene_state_handler>( this );
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
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F11 )
      {
        get_systems_event_queue().enqueue(
            Events::PlayerMortalityEvent( Cmp::PlayerMortality::State::SUICIDE, Utils::get_player_position( getReg() ) ) );
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad3 )
      {
        for ( auto [pc_entity, pc_cmp, pc_health_cmp] : getReg().view<Cmp::PlayableCharacter, Cmp::PlayerHealth>().each() )
        {
          pc_health_cmp.health = std::clamp( pc_health_cmp.health + 10, 0, 100 );
          SPDLOG_INFO( "Player gained health (player cheated)" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::NumpadDecimal )
      {
        for ( auto [pc_entity, pc_cmp, pc_health_cmp] : getReg().view<Cmp::PlayableCharacter, Cmp::PlayerHealth>().each() )
        {
          pc_health_cmp.health = std::clamp( pc_health_cmp.health - 10, 0, 100 );
          SPDLOG_INFO( "Player lost health (player cheated)" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad5 )
      {
        for ( auto [pc_entity, pc_cmp, pc_cadaver_count_cmp] : getReg().view<Cmp::PlayableCharacter, Cmp::PlayerCadaverCount>().each() )
        {
          pc_cadaver_count_cmp.increment_count( 1 );
          SPDLOG_INFO( "Player gained a cadaver (player cheated)" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F2 )
      {
        m_scenemanager_event_dispatcher.enqueue( Events::SceneManagerEvent( Events::SceneManagerEvent::Type::ENTER_CRYPT ) );
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

void SceneInputRouter::crypt_scene_state_handler()
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
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F9 )
      {
        for ( auto [_entt, _sys] : getReg().view<Cmp::System>().each() )
        {
          _sys.dark_mode_enabled = not _sys.dark_mode_enabled;
          SPDLOG_INFO( "Dark mode is now {}", _sys.dark_mode_enabled ? "ENABLED" : "DISABLED" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F11 )
      {
        // dont set PlayerMortality::State directly, instead update health/death_progress and let
        // the PlayerSystem logic handle it
        for ( auto [entity, pc_mort_cmp, pc_health_cmp] : getReg().view<Cmp::PlayerMortality, Cmp::PlayerHealth>().each() )
        {
          pc_health_cmp.health = 0;
          pc_mort_cmp.death_progress = 1.0f;
          SPDLOG_INFO( "Player committed suicide" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad3 )
      {
        for ( auto [pc_entity, pc_cmp, pc_health_cmp] : getReg().view<Cmp::PlayableCharacter, Cmp::PlayerHealth>().each() )
        {
          pc_health_cmp.health = std::clamp( pc_health_cmp.health + 10, 0, 100 );
          SPDLOG_INFO( "Player gained health (player cheated)" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::NumpadDecimal )
      {
        for ( auto [pc_entity, pc_cmp, pc_health_cmp] : getReg().view<Cmp::PlayableCharacter, Cmp::PlayerHealth>().each() )
        {
          pc_health_cmp.health = std::clamp( pc_health_cmp.health - 10, 0, 100 );
          SPDLOG_INFO( "Player lost health (player cheated)" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad5 )
      {
        for ( auto [pc_entity, pc_cmp, pc_cadaver_count_cmp] : getReg().view<Cmp::PlayableCharacter, Cmp::PlayerCadaverCount>().each() )
        {
          pc_cadaver_count_cmp.increment_count( 1 );
          SPDLOG_INFO( "Player gained a cadaver (player cheated)" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F2 )
      {
        m_scenemanager_event_dispatcher.enqueue( Events::SceneManagerEvent( Events::SceneManagerEvent::Type::EXIT_CRYPT ) );
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Escape )
      {
        // not implemented yet
      }
      else if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Key::E ) )
      {
        get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::ACTIVATE ) );
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::R )
      {
        get_systems_event_queue().trigger( Events::CryptRoomEvent( Events::CryptRoomEvent::Type::SHUFFLE_PASSAGES ) );
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::T )
      {
        get_systems_event_queue().trigger( Events::CryptRoomEvent( Events::CryptRoomEvent::Type::FINAL_PASSAGE ) );
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Y )
      {
        get_systems_event_queue().trigger( Events::CryptRoomEvent( Events::CryptRoomEvent::Type::EXIT_ALL_PASSAGES ) );
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

  if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Key::E ) )
  {
    get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::ACTIVATE ) );
  }
  if ( sf::Mouse::isButtonPressed( sf::Mouse::Button::Left ) )
  {
    // get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::DIG ) );
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