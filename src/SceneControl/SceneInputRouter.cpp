#include <Audio/SoundBank.hpp>
#include <Components/Direction.hpp>
#include <Components/Exit.hpp>
#include <Components/Persistent/ExitKeyRequirement.hpp>
#include <Components/Persistent/MaxNumAltars.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Player/PlayerBlastRadius.hpp>
#include <Components/Player/PlayerCadaverCount.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerHealth.hpp>
#include <Components/Player/PlayerKeysCount.hpp>
#include <Components/Player/PlayerMortality.hpp>
#include <Components/Position.hpp>
#include <Components/System.hpp>
#include <Events/CryptRoomEvent.hpp>
#include <Events/LoadSettingsEvent.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Events/PlayerMortalityEvent.hpp>
#include <Events/SaveSettingsEvent.hpp>
#include <Events/UnlockDoorEvent.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Player/PlayerWealth.hpp>
#include <SceneControl/Events/ProcessCryptSceneInputEvent.hpp>
#include <SceneControl/Events/ProcessGameoverSceneInputEvent.hpp>
#include <SceneControl/Events/ProcessGraveyardSceneInputEvent.hpp>
#include <SceneControl/Events/ProcessHolyWellSceneInputEvent.hpp>
#include <SceneControl/Events/ProcessLevelCompleteSceneInputEvent.hpp>
#include <SceneControl/Events/ProcessPausedMenuSceneInputEvent.hpp>
#include <SceneControl/Events/ProcessRuinSceneLowerInputEvent.hpp>
#include <SceneControl/Events/ProcessRuinSceneUpperInputEvent.hpp>
#include <SceneControl/Events/ProcessSettingsMenuSceneInputEvent.hpp>
#include <SceneControl/Events/ProcessShopSceneInputEvent.hpp>
#include <SceneControl/Events/ProcessTitleSceneInputEvent.hpp>
#include <SceneControl/Events/SceneManagerEvent.hpp>
#include <SceneControl/SceneInputRouter.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/SystemStore.hpp>
#include <Utils/Player.hpp>
#include <Utils/Utils.hpp>

#include <imgui-SFML.h>

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
  m_nav_event_dispatcher.sink<Events::ProcessHolyWellSceneInputEvent>().connect<&SceneInputRouter::holywell_scene_state_handler>( this );
  m_nav_event_dispatcher.sink<Events::ProcessRuinSceneLowerInputEvent>().connect<&SceneInputRouter::ruin_scene_state_handler>( this );
  m_nav_event_dispatcher.sink<Events::ProcessRuinSceneUpperInputEvent>().connect<&SceneInputRouter::ruin_scene_state_handler>( this );
  m_nav_event_dispatcher.sink<Events::ProcessShopSceneInputEvent>().connect<&SceneInputRouter::shop_scene_state_handler>( this );
  // clang-format on
}

void SceneInputRouter::title_scene_input_handler()
{
  using namespace sf::Keyboard;
  while ( const std::optional event = m_window.pollEvent() )
  {
    ImGui::SFML::ProcessEvent( m_window, *event );
    if ( event->is<sf::Event::Closed>() ) { m_window.close(); }
    else if ( const auto *resized = event->getIf<sf::Event::Resized>() ) { resize_window( resized->size ); }
    else if ( const auto *keyPressed = event->getIf<sf::Event::KeyPressed>() )
    {
      if ( keyPressed->scancode == sf::Keyboard::Scancode::Enter ) { enqueue( Events::SceneManagerEvent::Type::START_GAME ); }
      else if ( keyPressed->scancode == sf::Keyboard::Scancode::Q ) { enqueue( Events::SceneManagerEvent::Type::EXIT_GAME ); }
      else if ( keyPressed->scancode == sf::Keyboard::Scancode::S ) { enqueue( Events::SceneManagerEvent::Type::SETTINGS_MENU ); }
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
    else if ( const auto *resized = event->getIf<sf::Event::Resized>() ) { resize_window( resized->size ); }
    else if ( const auto *keyPressed = event->getIf<sf::Event::KeyPressed>() )
    {
      if ( keyPressed->scancode == sf::Keyboard::Scancode::Escape ) { enqueue( Events::SceneManagerEvent::Type::EXIT_SETTINGS_MENU ); }
      else if ( keyPressed->scancode == sf::Keyboard::Scancode::R ) { get_systems_event_queue().trigger( Events::LoadSettingsEvent() ); }
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
    else if ( const auto *resized = event->getIf<sf::Event::Resized>() ) { resize_window( resized->size ); }
    else if ( const auto *keyReleased = event->getIf<sf::Event::KeyReleased>() )
    {
      if ( keyReleased->scancode == sf::Keyboard::Scancode::F1 ) { toggle_collision_detection(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F2 ) { toggle_show_pathfinding(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F3 ) { toggle_show_debug(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F4 ) { toggle_show_nopath(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F5 )
      {
        enqueue( Events::SceneManagerEvent::Type::ENTER_CRYPT );
        // remember the player position
        Factory::add_player_last_graveyard_pos( getReg(), Utils::Player::get_position( getReg() ), { 0.f, 0.f } );
        // drop any inventory
        auto [inventory_entt, inventory_slot_type] = Utils::Player::get_inventory_type( getReg() );
        auto dropped_entt = Factory::drop_inventory_slot_into_world(
            getReg(), Utils::Player::get_position( getReg() ), m_sprite_factory.get_multisprite_by_type( inventory_slot_type ), inventory_entt );
        if ( dropped_entt != entt::null ) { m_sound_bank.get_effect( "drop_relic" ).play(); }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F6 )
      {
        enqueue( Events::SceneManagerEvent::Type::ENTER_HOLYWELL );
        // remember the player position
        Factory::add_player_last_graveyard_pos( getReg(), Utils::Player::get_position( getReg() ), { 0.f, 0.f } );
        // drop any inventory
        auto [inventory_entt, inventory_slot_type] = Utils::Player::get_inventory_type( getReg() );
        auto dropped_entt = Factory::drop_inventory_slot_into_world(
            getReg(), Utils::Player::get_position( getReg() ), m_sprite_factory.get_multisprite_by_type( inventory_slot_type ), inventory_entt );
        if ( dropped_entt != entt::null ) { m_sound_bank.get_effect( "drop_relic" ).play(); }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F7 )
      {
        enqueue( Events::SceneManagerEvent::Type::ENTER_RUIN_LOWER );
        // remember the player position
        Factory::add_player_last_graveyard_pos( getReg(), Utils::Player::get_position( getReg() ), { 0.f, 0.f } );
        // drop any inventory
        auto [inventory_entt, inventory_slot_type] = Utils::Player::get_inventory_type( getReg() );
        auto dropped_entt = Factory::drop_inventory_slot_into_world(
            getReg(), Utils::Player::get_position( getReg() ), m_sprite_factory.get_multisprite_by_type( inventory_slot_type ), inventory_entt );
        if ( dropped_entt != entt::null ) { m_sound_bank.get_effect( "drop_relic" ).play(); }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F8 )
      {
        enqueue( Events::SceneManagerEvent::Type::ENTER_SHOP );
        // remember the player position
        Factory::add_player_last_graveyard_pos( getReg(), Utils::Player::get_position( getReg() ), { 0.f, 0.f } );
        // drop any inventory
        auto [inventory_entt, inventory_slot_type] = Utils::Player::get_inventory_type( getReg() );
        auto dropped_entt = Factory::drop_inventory_slot_into_world(
            getReg(), Utils::Player::get_position( getReg() ), m_sprite_factory.get_multisprite_by_type( inventory_slot_type ), inventory_entt );
        if ( dropped_entt != entt::null ) { m_sound_bank.get_effect( "drop_relic" ).play(); }
      }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F11 ) { queue_suicide_event(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad1 ) { Utils::Player::get_blast_radius( getReg() ).value += 1; }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad2 ) { Utils::Player::get_health( getReg() ).health += 1; }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad3 ) { Utils::Player::get_wealth( getReg() ).wealth += 1; }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad4 ) { Utils::Player::get_cadaver_count( getReg() ).increment_count( 1 ); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Escape ) { enqueue( Events::SceneManagerEvent::Type::QUIT_GAME ); }
    }
    else if ( const auto *keyPressed = event->getIf<sf::Event::KeyPressed>() )
    {
      if ( keyPressed->scancode == sf::Keyboard::Scancode::P ) { enqueue( Events::SceneManagerEvent::Type::PAUSE_GAME ); }
    }
  }

  process_move_keys();

  if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Key::Space ) )
  {
    get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::DROP_BOMB ) );
  }
  if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Key::E ) )
  {
    get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::ACTIVATE ) );
    get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::DROP_CARRYITEM ) );
  }
  if ( sf::Mouse::isButtonPressed( sf::Mouse::Button::Left ) )
  {
    get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::ATTACK ) );
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
    else if ( const auto *resized = event->getIf<sf::Event::Resized>() ) { resize_window( resized->size ); }
    else if ( const auto *keyReleased = event->getIf<sf::Event::KeyReleased>() )
    {
      if ( keyReleased->scancode == sf::Keyboard::Scancode::F1 ) { toggle_collision_detection(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F2 ) { toggle_show_pathfinding(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F3 ) { toggle_show_debug(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F4 ) { toggle_show_nopath(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F9 ) { toggle_show_darkmode(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F11 ) { queue_suicide_event(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad1 ) { Utils::Player::get_blast_radius( getReg() ).value += 1; }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad2 ) { Utils::Player::get_health( getReg() ).health += 1; }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad3 ) { Utils::Player::get_wealth( getReg() ).wealth += 1; }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad4 ) { Utils::Player::get_cadaver_count( getReg() ).increment_count( 1 ); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Escape ) { enqueue( Events::SceneManagerEvent::Type::EXIT_CRYPT ); }
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
      if ( keyPressed->scancode == sf::Keyboard::Scancode::P ) { enqueue( Events::SceneManagerEvent::Type::PAUSE_GAME ); }
    }
  }

  process_move_keys();

  if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Key::E ) )
  {
    get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::ACTIVATE ) );
  }
  if ( sf::Mouse::isButtonPressed( sf::Mouse::Button::Left ) )
  {
    // get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::DIG ) );
  }
}

void SceneInputRouter::holywell_scene_state_handler()
{

  using namespace sf::Keyboard;
  while ( const std::optional event = m_window.pollEvent() )
  {
    ImGui::SFML::ProcessEvent( m_window, *event );
    if ( event->is<sf::Event::Closed>() ) { m_window.close(); }
    else if ( const auto *resized = event->getIf<sf::Event::Resized>() ) { resize_window( resized->size ); }
    else if ( const auto *keyReleased = event->getIf<sf::Event::KeyReleased>() )
    {
      if ( keyReleased->scancode == sf::Keyboard::Scancode::F1 ) { toggle_collision_detection(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F2 ) { toggle_show_pathfinding(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F3 ) { toggle_show_debug(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F4 ) { toggle_show_nopath(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F9 ) { toggle_show_darkmode(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F11 ) { queue_suicide_event(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad1 ) { Utils::Player::get_blast_radius( getReg() ).value += 1; }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad2 ) { Utils::Player::get_health( getReg() ).health += 1; }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad3 ) { Utils::Player::get_wealth( getReg() ).wealth += 1; }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad4 ) { Utils::Player::get_cadaver_count( getReg() ).increment_count( 1 ); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Escape ) { enqueue( Events::SceneManagerEvent::Type::QUIT_GAME ); }
    }
    else if ( const auto *keyPressed = event->getIf<sf::Event::KeyPressed>() )
    {
      if ( keyPressed->scancode == sf::Keyboard::Scancode::P ) { enqueue( Events::SceneManagerEvent::Type::PAUSE_GAME ); }
    }
  }

  process_move_keys();

  if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Key::E ) )
  {
    get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::ACTIVATE ) );
    get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::DROP_CARRYITEM ) );
  }
  if ( sf::Mouse::isButtonPressed( sf::Mouse::Button::Left ) )
  {
    get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::ATTACK ) );
  }
}

void SceneInputRouter::shop_scene_state_handler()
{

  using namespace sf::Keyboard;
  while ( const std::optional event = m_window.pollEvent() )
  {
    ImGui::SFML::ProcessEvent( m_window, *event );
    if ( event->is<sf::Event::Closed>() ) { m_window.close(); }
    else if ( const auto *resized = event->getIf<sf::Event::Resized>() ) { resize_window( resized->size ); }
    else if ( const auto *keyReleased = event->getIf<sf::Event::KeyReleased>() )
    {
      if ( keyReleased->scancode == sf::Keyboard::Scancode::F1 ) { toggle_collision_detection(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F2 ) { toggle_show_pathfinding(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F3 ) { toggle_show_debug(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F4 ) { toggle_show_nopath(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F9 ) { toggle_show_darkmode(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F11 ) { queue_suicide_event(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad1 ) { Utils::Player::get_blast_radius( getReg() ).value += 1; }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad2 ) { Utils::Player::get_health( getReg() ).health += 1; }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad3 ) { Utils::Player::get_wealth( getReg() ).wealth += 1; }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad4 ) { Utils::Player::get_cadaver_count( getReg() ).increment_count( 1 ); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Escape ) { enqueue( Events::SceneManagerEvent::Type::QUIT_GAME ); }
    }
    else if ( const auto *keyPressed = event->getIf<sf::Event::KeyPressed>() )
    {
      if ( keyPressed->scancode == sf::Keyboard::Scancode::P ) { enqueue( Events::SceneManagerEvent::Type::PAUSE_GAME ); }
    }
  }

  process_move_keys();

  if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Key::E ) )
  {
    get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::ACTIVATE ) );
    get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::DROP_CARRYITEM ) );
  }
  if ( sf::Mouse::isButtonPressed( sf::Mouse::Button::Left ) )
  {
    get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::ATTACK ) );
  }
}

void SceneInputRouter::ruin_scene_state_handler()
{

  using namespace sf::Keyboard;
  while ( const std::optional event = m_window.pollEvent() )
  {
    ImGui::SFML::ProcessEvent( m_window, *event );
    if ( event->is<sf::Event::Closed>() ) { m_window.close(); }
    else if ( const auto *resized = event->getIf<sf::Event::Resized>() ) { resize_window( resized->size ); }
    else if ( const auto *keyReleased = event->getIf<sf::Event::KeyReleased>() )
    {
      if ( keyReleased->scancode == sf::Keyboard::Scancode::F1 ) { toggle_collision_detection(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F2 ) { toggle_show_pathfinding(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F3 ) { toggle_show_debug(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F4 ) { toggle_show_nopath(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F9 ) { toggle_show_darkmode(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::F11 ) { queue_suicide_event(); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad1 ) { Utils::Player::get_blast_radius( getReg() ).value += 1; }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad2 ) { Utils::Player::get_health( getReg() ).health += 1; }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad3 ) { Utils::Player::get_wealth( getReg() ).wealth += 1; }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Numpad4 ) { Utils::Player::get_cadaver_count( getReg() ).increment_count( 1 ); }
      else if ( keyReleased->scancode == sf::Keyboard::Scancode::Escape ) { enqueue( Events::SceneManagerEvent::Type::QUIT_GAME ); }
    }
    else if ( const auto *keyPressed = event->getIf<sf::Event::KeyPressed>() )
    {
      if ( keyPressed->scancode == sf::Keyboard::Scancode::P ) { enqueue( Events::SceneManagerEvent::Type::PAUSE_GAME ); }
    }
  }

  process_move_keys();

  if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Key::E ) )
  {
    get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::ACTIVATE ) );
    get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::DROP_CARRYITEM ) );
  }
  if ( sf::Mouse::isButtonPressed( sf::Mouse::Button::Left ) )
  {
    get_systems_event_queue().trigger( Events::PlayerActionEvent( Events::PlayerActionEvent::GameActions::ATTACK ) );
  }
}

void SceneInputRouter::paused_scene_state_handler()
{

  using namespace sf::Keyboard;
  while ( const std::optional event = m_window.pollEvent() )
  {
    ImGui::SFML::ProcessEvent( m_window, *event );
    if ( event->is<sf::Event::Closed>() ) { m_window.close(); }
    else if ( const auto *resized = event->getIf<sf::Event::Resized>() ) { resize_window( resized->size ); }
    else if ( const auto *keyPressed = event->getIf<sf::Event::KeyPressed>() )
    {
      if ( keyPressed->scancode == sf::Keyboard::Scancode::P ) { enqueue( Events::SceneManagerEvent::Type::RESUME_GAME ); }
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
    else if ( const auto *resized = event->getIf<sf::Event::Resized>() ) { resize_window( resized->size ); }
    else if ( const auto *keyPressed = event->getIf<sf::Event::KeyPressed>() )
    {
      if ( keyPressed->scancode == sf::Keyboard::Scancode::R ) { enqueue( Events::SceneManagerEvent::Type::RETURN_TO_TITLE ); }
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
    else if ( const auto *resized = event->getIf<sf::Event::Resized>() ) { resize_window( resized->size ); }
    else if ( const auto *keyPressed = event->getIf<sf::Event::KeyPressed>() )
    {
      if ( keyPressed->scancode == sf::Keyboard::Scancode::R ) { enqueue( Events::SceneManagerEvent::Type::RETURN_TO_TITLE ); }
    }
  }
}

// PRIVATE

void SceneInputRouter::process_move_keys()
{
  // allow multiple changes to the direction vector, otherwise we get a delayed slurred movement
  auto player_direction_view = getReg().view<Cmp::PlayerCharacter, Cmp::Direction>();
  for ( auto [entity, player, direction] : player_direction_view.each() )
  {
    direction.x = 0;
    direction.y = 0;
    if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Key::W ) ) { direction.y = -1; } // move player up
    if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Key::A ) ) { direction.x = -1; } // move player left
    if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Key::D ) ) { direction.x = 1; }  // move player right
    if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Key::S ) ) { direction.y = 1; }  // move player down
  }
}

void SceneInputRouter::resize_window( sf::Vector2u size )
{
  sf::FloatRect visibleArea( { 0.f, 0.f }, sf::Vector2f( size ) );
  m_window.setView( sf::View( visibleArea ) );
}

void SceneInputRouter::toggle_collision_detection()
{
  for ( auto [entt, sys_cmp] : getReg().view<Cmp::System>().each() )
  {
    sys_cmp.collisions_disabled = not sys_cmp.collisions_disabled;
    SPDLOG_INFO( "Collisions are now {}", sys_cmp.collisions_disabled ? "DISABLED" : "ENABLED" );
  }
}

void SceneInputRouter::toggle_show_pathfinding()
{
  for ( auto [entt, sys_cmp] : getReg().view<Cmp::System>().each() )
  {
    sys_cmp.show_path_finding = not sys_cmp.show_path_finding;
    SPDLOG_INFO( "Show Pathfinding is now {}", sys_cmp.show_path_finding ? "ENABLED" : "DISABLED" );
  }
}

void SceneInputRouter::toggle_show_debug()
{
  for ( auto [entt, sys_cmp] : getReg().view<Cmp::System>().each() )
  {
    sys_cmp.show_debug_stats = not sys_cmp.show_debug_stats;
    SPDLOG_INFO( "Show debug stats is now {}", sys_cmp.show_debug_stats ? "ENABLED" : "DISABLED" );
  }
}

void SceneInputRouter::toggle_show_nopath()
{

  for ( auto [entt, sys_cmp] : getReg().view<Cmp::System>().each() )
  {
    if ( not sys_cmp.show_npcnopath and not sys_cmp.show_playernopath )
    {
      sys_cmp.show_npcnopath = true;
      sys_cmp.show_playernopath = false;
      SPDLOG_INFO( "Show NpcNoPathFinding is now ENABLED" );
    }
    else if ( sys_cmp.show_npcnopath )
    {
      sys_cmp.show_npcnopath = false;
      sys_cmp.show_playernopath = true;
      SPDLOG_INFO( "Show PlayerNoPath is now ENABLED" );
    }
    else
    {
      sys_cmp.show_npcnopath = false;
      sys_cmp.show_playernopath = false;
      SPDLOG_INFO( "Show PlayerNoPath and NpcNoPathFinding are now DISABLED" );
    }
  }
}

void SceneInputRouter::toggle_show_darkmode()
{
  for ( auto [_entt, _sys] : getReg().view<Cmp::System>().each() )
  {
    _sys.dark_mode_enabled = not _sys.dark_mode_enabled;
    SPDLOG_INFO( "Dark mode is now {}", _sys.dark_mode_enabled ? "ENABLED" : "DISABLED" );
  }
}

void SceneInputRouter::queue_suicide_event()
{
  get_systems_event_queue().enqueue( Events::PlayerMortalityEvent( Cmp::PlayerMortality::State::SUICIDE, Utils::Player::get_position( getReg() ) ) );
}

void SceneInputRouter::enqueue( Events::SceneManagerEvent::Type ev ) { m_scenemanager_event_dispatcher.enqueue( Events::SceneManagerEvent( ev ) ); }

} // namespace ProceduralMaze::Sys