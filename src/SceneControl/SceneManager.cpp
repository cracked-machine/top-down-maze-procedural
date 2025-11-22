#include <SceneControl/IScene.hpp>
#include <SceneControl/SceneManager.hpp>
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

void SceneManager::push( std::unique_ptr<IScene> scene )
{
  // call exit handler on the current scene
  if ( !m_scene_stack.empty() ) m_scene_stack.current().on_exit();

  // store callback pointer in the new scene and push to stack
  // new scene is now the current scene
  m_scene_stack.push( std::move( scene ) );

  SPDLOG_INFO( "Pushed {}.", m_scene_stack.current().get_name() );
  m_scene_stack.print_stack();

  // update the systems with current scene registry
  inject_registry();

  // call enter handler on the new scene. Use a loading screen to hide any delays
  m_scene_stack.current().on_init();
  loading_screen( [&]() { m_scene_stack.current().on_enter(); }, m_splash_texture );
}

void SceneManager::pop()
{
  if ( m_scene_stack.size() == 1 ) return;
  m_scene_stack.current().on_exit();
  SPDLOG_INFO( "Popping {}.", m_scene_stack.current().get_name() );
  m_scene_stack.pop();

  m_scene_stack.print_stack();

  inject_registry();
  if ( !m_scene_stack.empty() ) m_scene_stack.current().on_enter();
}

void SceneManager::push_overlay( std::unique_ptr<IScene> scene )
{
  // new scene is now the current scene
  m_scene_stack.push( std::move( scene ) );
  SPDLOG_INFO( "Pushed {} (overlay).  ", m_scene_stack.current().get_name() );
  m_scene_stack.print_stack();

  // call enter handler on the new scene.
  m_scene_stack.current().on_enter();
}

void SceneManager::pop_overlay()
{
  if ( m_scene_stack.size() == 1 ) return;
  m_scene_stack.current().on_exit();
  SPDLOG_INFO( "Popping {} (overlay).", m_scene_stack.current().get_name() );
  m_scene_stack.pop();

  m_scene_stack.print_stack();
}

void SceneManager::replace( std::unique_ptr<IScene> scene )
{
  if ( !m_scene_stack.empty() )
  {
    m_scene_stack.print_stack();
    m_scene_stack.current().on_exit();
    SPDLOG_INFO( "Popping {}.", m_scene_stack.current().get_name() );
    m_scene_stack.pop();
    m_scene_stack.print_stack();
  }

  m_scene_stack.push( std::move( scene ) );
  SPDLOG_INFO( "Pushed {}.", m_scene_stack.current().get_name() );
  m_scene_stack.print_stack();

  inject_registry();
  loading_screen( [&]() { m_scene_stack.current().on_enter(); }, m_splash_texture );
}

void SceneManager::replace_overlay( std::unique_ptr<IScene> scene )
{
  if ( !m_scene_stack.empty() )
  {
    m_scene_stack.print_stack();
    SPDLOG_INFO( "Replacing {} (overlay).", m_scene_stack.current().get_name() );
    m_scene_stack.pop();
    m_scene_stack.print_stack();
  }

  m_scene_stack.push( std::move( scene ) );
  SPDLOG_INFO( "Pushed {}.", m_scene_stack.current().get_name() );
  m_scene_stack.print_stack();

  inject_registry();
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
      replace_overlay( std::move( game_over_scene ) );
      break;
    }
    case Events::SceneManagerEvent::Type::LEVEL_COMPLETE:
    {
      SPDLOG_INFO( "SceneManager: Events::SceneManagerEvent::Type::LEVEL_COMPLETE requested" );
      auto level_complete_scene = std::make_unique<LevelCompleteScene>( m_sound_bank, m_system_store, m_nav_event_dispatcher );
      replace_overlay( std::move( level_complete_scene ) );
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

void SceneManager::inject_registry()
{
  if ( m_scene_stack.empty() ) { throw std::runtime_error( "SceneManager::inject_registry: No current scene available" ); }
  entt::registry &reg = m_scene_stack.current().get_registry();
  for ( auto &sys : m_system_store )
    sys.second->setReg( reg ); // pass the unique_ptr by reference

  SPDLOG_INFO( "Injected registry into {} systems for {}", m_system_store.size(), m_scene_stack.current().get_name() );
}

} // namespace ProceduralMaze::Scene