#include <Components/Exit.hpp>
#include <Components/LargeObstacle.hpp>
#include <Components/Persistent/MaxShrines.hpp>
#include <EventHandler.hpp>
#include <Events/SaveSettingsEvent.hpp>
#include <Events/UnlockDoorEvent.hpp>

namespace ProceduralMaze::Sys {

EventHandler::EventHandler( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &m_window,
                            Sprites::SpriteFactory &sprite_factory )
    : Sys::BaseSystem( reg, m_window, sprite_factory )
{
}

void EventHandler::menu_state_handler()
{
  auto &game_state = get_persistent_component<Cmp::Persistent::GameState>();

  using namespace sf::Keyboard;
  while ( const std::optional event = m_window.pollEvent() )
  {
    ImGui::SFML::ProcessEvent( m_window, *event );
    if ( event->is<sf::Event::Closed>() ) { game_state.current_state = Cmp::Persistent::GameState::State::EXITING; }
    else if ( const auto *resized = event->getIf<sf::Event::Resized>() )
    {
      sf::FloatRect visibleArea( { 0.f, 0.f }, sf::Vector2f( resized->size ) );
      m_window.setView( sf::View( visibleArea ) );
    }
    else if ( const auto *keyPressed = event->getIf<sf::Event::KeyPressed>() )
    {
      if ( keyPressed->scancode == sf::Keyboard::Scancode::Enter )
      {
        game_state.current_state = Cmp::Persistent::GameState::State::LOADING;
      }
      else if ( keyPressed->scancode == sf::Keyboard::Scancode::Q )
      {
        game_state.current_state = Cmp::Persistent::GameState::State::EXITING;
      }
      else if ( keyPressed->scancode == sf::Keyboard::Scancode::S )
      {
        game_state.current_state = Cmp::Persistent::GameState::State::SETTINGS;
      }
    }
  }
}

void EventHandler::settings_state_handler()
{
  auto &game_state = get_persistent_component<Cmp::Persistent::GameState>();
  using namespace sf::Keyboard;

  while ( const std::optional event = m_window.pollEvent() )
  {
    ImGui::SFML::ProcessEvent( m_window, *event );
    if ( event->is<sf::Event::Closed>() )
    {
      getEventDispatcher().trigger( Events::SaveSettingsEvent() );
      game_state.current_state = Cmp::Persistent::GameState::State::EXITING;
      return; // Exit immediately after state change
    }
    else if ( const auto *resized = event->getIf<sf::Event::Resized>() )
    {
      sf::FloatRect visibleArea( { 0.f, 0.f }, sf::Vector2f( resized->size ) );
      m_window.setView( sf::View( visibleArea ) );
    }
    else if ( const auto *keyPressed = event->getIf<sf::Event::KeyPressed>() )
    {
      if ( keyPressed->scancode == sf::Keyboard::Scancode::Escape )
      {
        getEventDispatcher().trigger( Events::SaveSettingsEvent() );
        game_state.current_state = Cmp::Persistent::GameState::State::MENU;
        return; // Exit immediately after state change
      }
    }
  }
}

void EventHandler::game_state_handler()
{
  auto &game_state = get_persistent_component<Cmp::Persistent::GameState>();
  using namespace sf::Keyboard;
  while ( const std::optional event = m_window.pollEvent() )
  {
    ImGui::SFML::ProcessEvent( m_window, *event );
    if ( event->is<sf::Event::Closed>() ) { game_state.current_state = Cmp::Persistent::GameState::State::EXITING; }
    else if ( const auto *resized = event->getIf<sf::Event::Resized>() )
    {
      sf::FloatRect visibleArea( { 0.f, 0.f }, sf::Vector2f( resized->size ) );
      m_window.setView( sf::View( visibleArea ) );
    }
    else if ( const auto *keyReleased = event->getIf<sf::Event::KeyReleased>() )
    {
      if ( keyReleased->scancode == sf::Keyboard::Scancode::F1 )
      {
        for ( auto [_entt, _sys] : m_reg->view<Cmp::System>().each() )
        {
          _sys.collisions_enabled = not _sys.collisions_enabled;
          SPDLOG_INFO( "Collisions are now {}", _sys.collisions_enabled ? "ENABLED" : "DISABLED" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F2 )
      {
        for ( auto [_entt, _sys] : m_reg->view<Cmp::System>().each() )
        {
          _sys.pause_flood = not _sys.pause_flood;
          SPDLOG_INFO( "Pause flood is now {}", _sys.pause_flood ? "ENABLED" : "DISABLED" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F3 )
      {
        for ( auto [_entt, _sys] : m_reg->view<Cmp::System>().each() )
        {
          _sys.show_path_distances = not _sys.show_path_distances;
          SPDLOG_INFO( "Show player distances is now {}", _sys.show_path_distances ? "ENABLED" : "DISABLED" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F4 )
      {
        for ( auto [_entt, _sys] : m_reg->view<Cmp::System>().each() )
        {
          _sys.show_armed_obstacles = not _sys.show_armed_obstacles;
          SPDLOG_INFO( "Show armed obstacles is now {}", _sys.show_armed_obstacles ? "ENABLED" : "DISABLED" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F5 )
      {
        for ( auto [_entt, _sys] : m_reg->view<Cmp::System>().each() )
        {
          _sys.show_debug_stats = not _sys.show_debug_stats;
          SPDLOG_INFO( "Show debug stats is now {}", _sys.show_debug_stats ? "ENABLED" : "DISABLED" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F10 )
      {
        auto max_num_shrines = get_persistent_component<Cmp::Persistent::MaxShrines>();
        auto lo_view = m_reg->view<Cmp::LargeObstacle>();
        for ( auto [lo_entt, lo_cmp] : lo_view.each() )
        {
          if ( lo_cmp.getType() == "SHRINE" && not lo_cmp.are_powers_active() )
          {
            lo_cmp.set_powers_active();
            SPDLOG_DEBUG( "Activating shrine at entity {}", static_cast<int>( lo_entt ) );
          }
        }
        auto exit_cmp = m_reg->view<Cmp::Exit>();
        for ( auto [exit_entt, exit_cmp] : exit_cmp.each() )
        {
          // exit already unlocked
          if ( exit_cmp.m_locked == false ) continue;
        }
        getEventDispatcher().trigger( Events::UnlockDoorEvent() );
        SPDLOG_INFO( "Player cheated and unlocked exit" );
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F11 )
      {
        for ( auto [_, _pc] : m_reg->view<Cmp::PlayableCharacter>().each() )
        {
          _pc.alive = false;
          SPDLOG_INFO( "Player committed suicide" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F12 )
      {
        for ( auto [_ent, _sys] : m_reg->view<Cmp::System>().each() )
        {
          _sys.level_complete = true;
          SPDLOG_INFO( "Level complete (player cheated)" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::M )
      {
        for ( auto [_ent, _sys] : m_reg->view<Cmp::System>().each() )
        {
          _sys.minimap_enabled = not _sys.minimap_enabled;
          SPDLOG_INFO( "Minimap enabled (player cheated)" );
        }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Escape )
      {
        game_state.current_state = Cmp::Persistent::GameState::State::UNLOADING;
      }
    }
    else if ( const auto *keyPressed = event->getIf<sf::Event::KeyPressed>() )
    {
      if ( keyPressed->scancode == sf::Keyboard::Scancode::P )
      {
        using namespace std::chrono_literals;
        game_state.current_state = Cmp::Persistent::GameState::State::PAUSED;
        SPDLOG_INFO( "Pausing game...." );
        std::this_thread::sleep_for( 200ms );
      }
    }
  }

  // allow multiple changes to the direction vector, otherwise we get a
  // delayed slurred movement
  auto player_direction_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Direction>();
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
    getEventDispatcher().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::DROP_BOMB ) );
  }
  if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Key::E ) )
  {
    getEventDispatcher().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::ACTIVATE ) );
  }
  if ( sf::Mouse::isButtonPressed( sf::Mouse::Button::Left ) )
  {
    getEventDispatcher().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::DIG ) );
  }
}

void EventHandler::paused_state_handler()
{
  auto &game_state = get_persistent_component<Cmp::Persistent::GameState>();
  using namespace sf::Keyboard;
  while ( const std::optional event = m_window.pollEvent() )
  {
    ImGui::SFML::ProcessEvent( m_window, *event );
    if ( event->is<sf::Event::Closed>() ) { game_state.current_state = Cmp::Persistent::GameState::State::EXITING; }
    else if ( const auto *resized = event->getIf<sf::Event::Resized>() )
    {
      sf::FloatRect visibleArea( { 0.f, 0.f }, sf::Vector2f( resized->size ) );
      m_window.setView( sf::View( visibleArea ) );
    }
    else if ( const auto *keyPressed = event->getIf<sf::Event::KeyPressed>() )
    {
      if ( keyPressed->scancode == sf::Keyboard::Scancode::P )
      {
        game_state.current_state = Cmp::Persistent::GameState::State::PLAYING;
      }
    }
  }
}

void EventHandler::game_over_state_handler()
{
  auto &game_state = get_persistent_component<Cmp::Persistent::GameState>();
  using namespace sf::Keyboard;
  while ( const std::optional event = m_window.pollEvent() )
  {
    ImGui::SFML::ProcessEvent( m_window, *event );
    if ( event->is<sf::Event::Closed>() ) { game_state.current_state = Cmp::Persistent::GameState::State::EXITING; }
    else if ( const auto *resized = event->getIf<sf::Event::Resized>() )
    {
      sf::FloatRect visibleArea( { 0.f, 0.f }, sf::Vector2f( resized->size ) );
      m_window.setView( sf::View( visibleArea ) );
    }
    else if ( const auto *keyPressed = event->getIf<sf::Event::KeyPressed>() )
    {
      if ( keyPressed->scancode == sf::Keyboard::Scancode::R )
      {
        game_state.current_state = Cmp::Persistent::GameState::State::UNLOADING;
      }
    }
  }
}

} // namespace ProceduralMaze::Sys