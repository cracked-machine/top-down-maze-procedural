#include <Scene/GraveyardScene.hpp>
#include <Scene/IScene.hpp>
#include <Scene/MainMenuScene.hpp>
#include <Scene/PausedMenuScene.hpp>
#include <Scene/SceneManager.hpp>
#include <Scene/SettingsMenuScene.hpp>

namespace ProceduralMaze::Scene
{

void SceneManager::update( sf::Time dt )
{
  if ( m_scenes.empty() ) return;

  // update from top down, stopping if a scene blocks updates
  for ( int i = static_cast<int>( m_scenes.size() ) - 1; i >= 0; --i )
  {
    m_scenes[i]->update( dt );

    if ( m_scenes[i]->blocks_update() ) break;
  }

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
  SPDLOG_INFO( "Stack size after push: {}", m_scenes.size() );

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
  m_scenes.pop_back();
  SPDLOG_INFO( "Stack size after pop: {}", m_scenes.size() );

  inject_registry();
  if ( !m_scenes.empty() ) m_scenes.back()->on_enter();
}

void SceneManager::push_overlay( std::unique_ptr<IScene> scene )
{
  // new scene is now the current scene
  scene->set_scene_manager( this );
  m_scenes.push_back( std::move( scene ) );
  SPDLOG_INFO( "Stack size after push: {}", m_scenes.size() );

  // call enter handler on the new scene.
  m_scenes.back()->on_enter();
}

void SceneManager::pop_overlay()
{
  if ( m_scenes.empty() ) return;
  m_scenes.back()->on_exit();
  m_scenes.pop_back();
  SPDLOG_INFO( "Stack size after pop: {}", m_scenes.size() );
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

void SceneManager::gen_level()
{
  m_scene_di_sys_ptrs.random_level_sys->generate();
  m_scene_di_sys_ptrs.cellauto_parser->set_random_level_generator( m_scene_di_sys_ptrs.random_level_sys );
  m_scene_di_sys_ptrs.cellauto_parser->iterate( 5 );
}

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
      case SceneRequest::StartGame:
      {
        // inject the dependencies into the new scene
        auto graveyard_scene = std::make_unique<GraveyardScene>(
            m_sound_bank, m_scene_di_sys_ptrs.persistent_sys, m_scene_di_sys_ptrs.player_sys, m_scene_di_sys_ptrs.render_game_sys,
            m_scene_di_sys_ptrs.event_handler, m_scene_di_sys_ptrs.anim_sys, m_scene_di_sys_ptrs.sinkhole_sys,
            m_scene_di_sys_ptrs.corruption_sys, m_scene_di_sys_ptrs.bomb_sys, m_scene_di_sys_ptrs.exit_sys,
            m_scene_di_sys_ptrs.loot_sys, m_scene_di_sys_ptrs.npc_sys, m_scene_di_sys_ptrs.wormhole_sys,
            m_scene_di_sys_ptrs.digging_sys, m_scene_di_sys_ptrs.footstep_sys, m_scene_di_sys_ptrs.path_find_sys,
            m_scene_di_sys_ptrs.render_overlay_sys, m_scene_di_sys_ptrs.render_player_sys, m_scene_di_sys_ptrs.random_level_sys,
            m_scene_di_sys_ptrs.cellauto_parser );

        // initialise registry and the dependencies
        push( std::move( graveyard_scene ) );

        break;
      }
      case SceneRequest::Pause:
      {
        auto paused_scene = std::make_unique<PausedMenuScene>( m_sound_bank, m_scene_di_sys_ptrs.persistent_sys,
                                                               m_scene_di_sys_ptrs.event_handler,
                                                               m_scene_di_sys_ptrs.render_menu_sys );
        push_overlay( std::move( paused_scene ) );
        break;
      }
      case SceneRequest::Resume:
      {
        pop_overlay();
        break;
      }
      case SceneRequest::GameOver:
      {
        pop();
        break;
      }
      case SceneRequest::LevelComplete:
      {
        pop();
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