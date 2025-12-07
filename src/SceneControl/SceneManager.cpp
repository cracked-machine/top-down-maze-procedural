#include <Components/PlayerCandlesCount.hpp>
#include <Components/PlayerKeysCount.hpp>
#include <Components/PlayerRelicCount.hpp>
#include <SceneControl/IScene.hpp>
#include <SceneControl/SceneManager.hpp>
#include <SceneControl/Scenes/CryptScene.hpp>
#include <SceneControl/Scenes/GameOverScene.hpp>
#include <SceneControl/Scenes/GraveyardScene.hpp>
#include <SceneControl/Scenes/LevelCompleteScene.hpp>
#include <SceneControl/Scenes/PausedMenuScene.hpp>
#include <SceneControl/Scenes/SettingsMenuScene.hpp>
#include <SceneControl/Scenes/TitleScene.hpp>

namespace ProceduralMaze::Scene
{

void SceneManager::update( sf::Time dt )
{

  if ( m_scene_stack.empty() ) return;
  m_scene_stack.current().update( dt );

  // Now safely consume any deferred events outside of Scene::update()
  if ( m_nav_event_dispatcher.size() > 0 ) { m_nav_event_dispatcher.update(); }
  if ( m_scenemanager_event_dispatcher.size() > 0 ) { m_scenemanager_event_dispatcher.update(); }
}

void SceneManager::push( std::unique_ptr<IScene> scene, ComponentTransfer::CopyRegistry clone_options )
{
  ComponentTransfer::RegistrySnapshot reg_snapshot = nullptr;

  // exit the current scene
  if ( !m_scene_stack.empty() )
  {
    reg_snapshot = m_cmp_transfer.capture( m_scene_stack.current(), clone_options );
    m_scene_stack.current().on_exit();
  }

  // new scene is now the current scene
  m_scene_stack.push( std::move( scene ) );

  SPDLOG_INFO( "Pushed {}.", m_scene_stack.current().get_name() );
  m_scene_stack.print_stack();

  // update the systems with current scene registry
  inject_current_scene_registry_into_systems();

  // // Transfer registry components - specify which component types to transfer
  // if ( reg_snapshot && clone_options == ComponentTransfer::CopyRegistry::FULL )
  // {
  //   m_cmp_transfer.transfer_player_entity( *reg_snapshot, m_scene_stack.current().get_registry() );
  // }

  // call enter handler on the new scene. Use a loading screen to hide any delays
  loading_screen( [&]() { m_scene_stack.current().on_init(); }, m_splash_texture );
  m_scene_stack.current().on_enter();
}

void SceneManager::pop( ComponentTransfer::CopyRegistry clone_options )
{
  ComponentTransfer::RegistrySnapshot reg_snapshot = m_cmp_transfer.capture( m_scene_stack.current(), clone_options );

  if ( m_scene_stack.size() == 1 ) return;
  m_scene_stack.current().on_exit();

  SPDLOG_INFO( "Popping {}.", m_scene_stack.current().get_name() );
  m_scene_stack.pop();
  m_scene_stack.print_stack();

  inject_current_scene_registry_into_systems();

  // // Transfer registry components - specify which component types to transfer
  // if ( reg_snapshot && clone_options == ComponentTransfer::CopyRegistry::FULL )
  // {
  //   loading_screen( [&]() { m_cmp_transfer.transfer_player_entity( *reg_snapshot, m_scene_stack.current().get_registry() ); }, m_splash_texture );
  // }

  if ( !m_scene_stack.empty() ) m_scene_stack.current().on_enter();
}

void SceneManager::push_overlay( std::unique_ptr<IScene> scene, ComponentTransfer::CopyRegistry clone_options )
{
  ComponentTransfer::RegistrySnapshot reg_snapshot = m_cmp_transfer.capture( m_scene_stack.current(), clone_options );

  m_scene_stack.push( std::move( scene ) );
  SPDLOG_INFO( "Pushed {} (overlay).  ", m_scene_stack.current().get_name() );
  m_scene_stack.print_stack();

  inject_current_scene_registry_into_systems();

  loading_screen( [&]() { m_scene_stack.current().on_init(); }, m_splash_texture );

  // Transfer components from the clone to the new top scene registry
  if ( reg_snapshot && clone_options == ComponentTransfer::CopyRegistry::FULL )
  {
    m_cmp_transfer.transfer_player_entity( *reg_snapshot, m_scene_stack.current().get_registry() );
  }
  m_scene_stack.current().on_enter();
}

void SceneManager::pop_overlay( ComponentTransfer::CopyRegistry clone_options )
{
  if ( m_scene_stack.size() == 1 ) return;

  ComponentTransfer::RegistrySnapshot reg_snapshot = m_cmp_transfer.capture( m_scene_stack.current(), clone_options );

  m_scene_stack.current().on_exit();

  // now pop the overlay scene from the top
  SPDLOG_INFO( "Popping {} (overlay).", m_scene_stack.current().get_name() );
  m_scene_stack.pop();
  m_scene_stack.print_stack();

  inject_current_scene_registry_into_systems();

  // Transfer components from the clone to the new top scene registry
  if ( reg_snapshot && clone_options == ComponentTransfer::CopyRegistry::FULL )
  {
    m_cmp_transfer.transfer_player_entity( *reg_snapshot, m_scene_stack.current().get_registry() );
  }

  m_scene_stack.current().on_enter();
}

void SceneManager::replace( std::unique_ptr<IScene> scene, ComponentTransfer::CopyRegistry clone_options )
{
  ComponentTransfer::RegistrySnapshot reg_snapshot = m_cmp_transfer.capture( m_scene_stack.current(), clone_options );

  // exit the current scene before popping
  m_scene_stack.print_stack();
  m_scene_stack.current().on_exit();

  // pop the current scene
  SPDLOG_INFO( "Popping {}.", m_scene_stack.current().get_name() );
  m_scene_stack.pop();
  m_scene_stack.print_stack();

  // push a new scene in its place
  m_scene_stack.push( std::move( scene ) );
  SPDLOG_INFO( "Pushed {}.", m_scene_stack.current().get_name() );
  m_scene_stack.print_stack();

  inject_current_scene_registry_into_systems();

  // // Transfer components from the clone to the new scene registry
  // if ( reg_snapshot && clone_options == ComponentTransfer::CopyRegistry::FULL )
  // {
  //   m_cmp_transfer.transfer_player_entity( *reg_snapshot, m_scene_stack.current().get_registry() );
  // }

  loading_screen( [&]() { m_scene_stack.current().on_enter(); }, m_splash_texture );
}

void SceneManager::replace_overlay( std::unique_ptr<IScene> scene, ComponentTransfer::CopyRegistry clone_options )
{
  ComponentTransfer::RegistrySnapshot reg_snapshot = m_cmp_transfer.capture( m_scene_stack.current(), clone_options );

  m_scene_stack.print_stack();
  SPDLOG_INFO( "Replacing {} (overlay).", m_scene_stack.current().get_name() );

  // pop the current overlay scene - do not call on_exit()
  m_scene_stack.pop();
  m_scene_stack.print_stack();

  // push a new scene in its place
  m_scene_stack.push( std::move( scene ) );
  SPDLOG_INFO( "Pushed {}.", m_scene_stack.current().get_name() );
  m_scene_stack.print_stack();

  inject_current_scene_registry_into_systems();

  // // Transfer components from the clone to the new scene registry
  // if ( reg_snapshot && clone_options == ComponentTransfer::CopyRegistry::FULL )
  // {
  //   m_cmp_transfer.transfer_player_entity( *reg_snapshot, m_scene_stack.current().get_registry() );
  // }

  loading_screen( [&]() { m_scene_stack.current().on_enter(); }, m_splash_texture );
}

void SceneManager::handle_events( const Events::SceneManagerEvent &event )
{
  switch ( event.m_type )
  {
    case Events::SceneManagerEvent::Type::START_GAME:
    {
      SPDLOG_INFO( "SceneManager: Events::SceneManagerEvent::Type::START_GAME requested" );
      auto graveyard_scene = std::make_unique<GraveyardScene>( m_sound_bank, m_system_store, m_nav_event_dispatcher );
      push( std::move( graveyard_scene ) );
      break;
    }
    case Events::SceneManagerEvent::Type::ENTER_CRYPT:
    {
      SPDLOG_INFO( "SceneManager: Events::SceneManagerEvent::Type::ENTER_CRYPT requested" );
      auto crypt_scene = std::make_unique<CryptScene>( m_sound_bank, m_system_store, m_nav_event_dispatcher );
      push_overlay( std::move( crypt_scene ), ComponentTransfer::CopyRegistry::FULL );
      break;
    }
    case Events::SceneManagerEvent::Type::EXIT_CRYPT:
    {
      SPDLOG_INFO( "SceneManager: Events::SceneManagerEvent::Type::EXIT_CRYPT requested" );
      pop_overlay( ComponentTransfer::CopyRegistry::FULL );
      break;
    }
    case Events::SceneManagerEvent::Type::EXIT_GAME:
    {
      SPDLOG_INFO( "SceneManager: Events::SceneManagerEvent::Type::EXIT_GAME requested" );
      m_window.close();
      break;
    }
    case Events::SceneManagerEvent::Type::SETTINGS_MENU:
    {
      SPDLOG_INFO( "SceneManager: Events::SceneManagerEvent::Type::SETTINGS_MENU requested" );
      auto settings_scene = std::make_unique<SettingsMenuScene>( m_system_store, m_nav_event_dispatcher );
      push( std::move( settings_scene ) );
      break;
    }
    case Events::SceneManagerEvent::Type::EXIT_SETTINGS_MENU:
    {
      SPDLOG_INFO( "SceneManager: Events::SceneManagerEvent::Type::EXIT_SETTINGS_MENU requested" );
      pop();
      break;
    }
    case Events::SceneManagerEvent::Type::QUIT_GAME:
    {
      SPDLOG_INFO( "SceneManager: Events::SceneManagerEvent::Type::QUIT_GAME requested" );
      pop();
      break;
    }
    case Events::SceneManagerEvent::Type::PAUSE_GAME:
    {
      SPDLOG_INFO( "SceneManager: Events::SceneManagerEvent::Type::PAUSE_GAME requested" );
      auto paused_scene = std::make_unique<PausedMenuScene>( m_sound_bank, m_system_store, m_nav_event_dispatcher );
      push_overlay( std::move( paused_scene ) );
      break;
    }
    case Events::SceneManagerEvent::Type::RESUME_GAME:
    {
      SPDLOG_INFO( "SceneManager: Events::SceneManagerEvent::Type::RESUME_GAME requested" );
      pop_overlay();
      break;
    }
    case Events::SceneManagerEvent::Type::GAME_OVER:
    {
      SPDLOG_INFO( "SceneManager: Events::SceneManagerEvent::Type::GAME_OVER requested" );
      auto game_over_scene = std::make_unique<GameOverScene>( m_sound_bank, m_system_store, m_nav_event_dispatcher );
      replace_overlay( std::move( game_over_scene ), ComponentTransfer::CopyRegistry::FULL );
      break;
    }
    case Events::SceneManagerEvent::Type::LEVEL_COMPLETE:
    {
      SPDLOG_INFO( "SceneManager: Events::SceneManagerEvent::Type::LEVEL_COMPLETE requested" );
      auto level_complete_scene = std::make_unique<LevelCompleteScene>( m_sound_bank, m_system_store, m_nav_event_dispatcher );
      replace_overlay( std::move( level_complete_scene ), ComponentTransfer::CopyRegistry::FULL );
      break;
    }
    case Events::SceneManagerEvent::Type::RETURN_TO_TITLE:
    {
      SPDLOG_INFO( "SceneManager: Events::SceneManagerEvent::Type::RETURN_TO_TITLE requested" );
      pop_overlay();
      break;
    }
    default:
      break;
  }
}

// PRIVATE FUNCTIONS

void SceneManager::inject_current_scene_registry_into_systems()
{
  if ( m_scene_stack.empty() ) { throw std::runtime_error( "SceneManager::inject_registry: No current scene available" ); }
  entt::registry &reg = m_scene_stack.current().get_registry();
  for ( auto &sys : m_system_store )
    sys.second->setReg( reg ); // pass the unique_ptr by reference

  SPDLOG_INFO( "Injected registry into {} systems for {}", m_system_store.size(), m_scene_stack.current().get_name() );
}

} // namespace ProceduralMaze::Scene