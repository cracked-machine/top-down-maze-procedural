#include <Scene/MainMenuScene.hpp>
#include <Scene/SceneManager.hpp>
#include <Scene/SettingsMenuScene.hpp>

namespace ProceduralMaze::Scene
{

void SceneManager::update( sf::Time dt )
{
  if ( m_scenes.empty() ) return;

  IScene *top = m_scenes.back().get();
  top->update( dt );

  // Handle request after update completes
  SceneRequest req = top->consume_request();
  if ( req != SceneRequest::None ) { handle_request( req ); }
}

void SceneManager::push( std::unique_ptr<IScene> scene )
{
  if ( !m_scenes.empty() ) m_scenes.back()->on_exit();
  scene->set_scene_manager( this );
  m_scenes.push_back( std::move( scene ) );

  inject_registry();
  m_scenes.back()->on_enter();
}

void SceneManager::pop()
{
  if ( m_scenes.empty() ) return;
  m_scenes.back()->on_exit();
  m_scenes.pop_back();

  inject_registry();
  if ( !m_scenes.empty() ) m_scenes.back()->on_enter();
}

void SceneManager::replace( std::unique_ptr<IScene> scene )
{
  if ( !m_scenes.empty() )
  {
    m_scenes.back()->on_exit();
    m_scenes.pop_back();
  }

  scene->set_scene_manager( this );
  m_scenes.push_back( std::move( scene ) );

  inject_registry();
  scene->on_enter();
}

IScene *SceneManager::current() { return m_scenes.empty() ? nullptr : m_scenes.back().get(); }

void SceneManager::handle_request( SceneRequest req )
{
  {

    switch ( req )
    {
      case SceneRequest::OpenSettings:
      {
        auto settings_scene = std::make_unique<SettingsMenuScene>(
            m_scene_di_sys_ptrs.persistent_sys, m_scene_di_sys_ptrs.render_menu_sys, m_scene_di_sys_ptrs.event_handler );
        push( std::move( settings_scene ) );
        break;
      }
      case SceneRequest::Pop:
      {
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
  entt::registry *reg = scene ? scene->get_registry() : nullptr;

  for ( auto *sys : m_reg_inject_system_ptrs )
    sys->setRegistry( reg );

  SPDLOG_INFO( "Injected registry into all systems for current scene." );
}

} // namespace ProceduralMaze::Scene