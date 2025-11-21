#include <Scene/GameOverScene.hpp>
#include <Scene/GraveyardScene.hpp>
#include <Scene/IScene.hpp>
#include <Scene/LevelCompleteScene.hpp>
#include <Scene/PausedMenuScene.hpp>
#include <Scene/SceneManager.hpp>
#include <Scene/SettingsMenuScene.hpp>
#include <Scene/TitleScene.hpp>
#include <cstddef>

namespace ProceduralMaze::Scene
{

void SceneManager::update( sf::Time dt )
{
  if ( m_scenes.empty() ) return;
  m_scenes.back()->update( dt );

  // Handle request after update completes
  SceneRequest req = m_scenes.back()->consume_request();
  if ( req != SceneRequest::None ) { handle_request( req ); }
}

void SceneManager::push( std::unique_ptr<IScene> scene )
{
  // call exit handler on the current scene
  if ( !m_scenes.empty() ) m_scenes.back()->on_exit();

  // store callback pointer in the new scene and push to stack
  // new scene is now the current scene
  scene->set_scene_manager( this );
  m_scenes.push_back( std::move( scene ) );

  SPDLOG_INFO( "Pushed {}.", m_scenes.back()->get_name() );
  print_stack();

  // update the systems with current scene registry
  inject_registry();

  // call enter handler on the new scene. Use a loading screen to hide any delays
  m_scenes.back()->on_init();
  loading_screen( [&]() { m_scenes.back()->on_enter(); }, m_splash_texture );
}

void SceneManager::pop()
{
  if ( m_scenes.empty() ) return;
  m_scenes.back()->on_exit();
  SPDLOG_INFO( "Popping {}.", m_scenes.back()->get_name() );
  m_scenes.pop_back();

  print_stack();

  inject_registry();
  if ( !m_scenes.empty() ) m_scenes.back()->on_enter();
}

void SceneManager::push_overlay( std::unique_ptr<IScene> scene )
{
  // new scene is now the current scene
  scene->set_scene_manager( this );
  m_scenes.push_back( std::move( scene ) );
  SPDLOG_INFO( "Pushed {} (overlay).  ", m_scenes.back()->get_name() );
  print_stack();

  // call enter handler on the new scene.
  m_scenes.back()->on_enter();
}

void SceneManager::pop_overlay()
{
  if ( m_scenes.empty() ) return;
  m_scenes.back()->on_exit();
  SPDLOG_INFO( "Popping {} (overlay).", m_scenes.back()->get_name() );
  m_scenes.pop_back();

  print_stack();
}

void SceneManager::replace( std::unique_ptr<IScene> scene )
{
  if ( !m_scenes.empty() )
  {
    print_stack();
    m_scenes.back()->on_exit();
    SPDLOG_INFO( "Popping {}.", m_scenes.back()->get_name() );
    m_scenes.pop_back();
    print_stack();
  }

  scene->set_scene_manager( this );
  m_scenes.push_back( std::move( scene ) );
  SPDLOG_INFO( "Pushed {}.", m_scenes.back()->get_name() );
  print_stack();

  inject_registry();
  loading_screen( [&]() { m_scenes.back()->on_enter(); }, m_splash_texture );
}

void SceneManager::replace_overlay( std::unique_ptr<IScene> scene )
{
  if ( !m_scenes.empty() )
  {
    print_stack();
    SPDLOG_INFO( "Replacing {} (overlay).", m_scenes.back()->get_name() );
    m_scenes.pop_back();
    print_stack();
  }

  scene->set_scene_manager( this );
  m_scenes.push_back( std::move( scene ) );
  SPDLOG_INFO( "Pushed {}.", m_scenes.back()->get_name() );
  print_stack();

  inject_registry();
  loading_screen( [&]() { m_scenes.back()->on_enter(); }, m_splash_texture );
}

IScene *SceneManager::current() { return m_scenes.empty() ? nullptr : m_scenes.back().get(); }

void SceneManager::handle_request( SceneRequest req )
{
  {
    switch ( req )
    {
      case SceneRequest::SettingsMenu:
      {
        auto settings_scene = std::make_unique<SettingsMenuScene>( m_system_store );
        push( std::move( settings_scene ) );
        break;
      }
      case SceneRequest::GraveyardScene:
      {
        auto graveyard_scene = std::make_unique<GraveyardScene>( m_sound_bank, m_system_store );
        push( std::move( graveyard_scene ) );

        break;
      }
      case SceneRequest::PausedMenu:
      {
        auto paused_scene = std::make_unique<PausedMenuScene>( m_sound_bank, m_system_store );
        push_overlay( std::move( paused_scene ) );
        break;
      }
      case SceneRequest::GameOver:
      {
        auto game_over_scene = std::make_unique<GameOverScene>( m_sound_bank, m_system_store );
        replace_overlay( std::move( game_over_scene ) );
        break;
      }
      case SceneRequest::LevelComplete:
      {
        auto level_complete_scene = std::make_unique<LevelCompleteScene>( m_sound_bank, m_system_store );
        replace_overlay( std::move( level_complete_scene ) );
        break;
      }
      case SceneRequest::PopOverlay:
      {
        // scene can request to pop itself but new top scene does NOT call on_enter or inject registry
        pop_overlay();
        break;
      }
      case SceneRequest::Pop:
      {
        // scene can request to pop itself and new top scene WILL call on_enter and inject registry
        pop();
        break;
      }
      case SceneRequest::Quit:
      {
        m_window.close();
        break;
      }
      default:
        break;
    }
  }
}

// PRIVATE FUNCTIONS

void SceneManager::inject_registry()
{
  auto *scene = current();
  entt::registry &reg = scene->get_registry();
  for ( auto &sys : m_system_store )
    sys.second->setReg( reg ); // pass the unique_ptr by reference

  SPDLOG_INFO( "Injected registry into {} systems for {}", m_system_store.size(), scene->get_name() );
}

void SceneManager::print_stack()
{
  std::stringstream ss;
  for ( std::size_t i = 0; i < m_scenes.size(); ++i )
  {
    auto s = m_scenes[i].get();
    ss << "[" << i << "] " << s->get_name() << " ";
  }
  ss << "(top)";
  SPDLOG_INFO( "{}", ss.str() );
}

} // namespace ProceduralMaze::Scene